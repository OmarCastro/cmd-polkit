#define _GNU_SOURCE
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <gmodule.h>
#include <time.h>
#include <stdint.h>
#include <fcntl.h>
#include "logger.h"
#include "app.h"
#include "json-glib.extension.h"
#include "accepted-actions.enum.h"
#include "error-message.dialog.h"
#include "polkit-auth-handler.service.h"


G_DEFINE_TYPE(CmdPkAgentPolkitListener, cmd_pk_agent_polkit_listener, POLKIT_AGENT_TYPE_LISTENER);



typedef struct _AuthDlgData AuthDlgData;
struct _AuthDlgData {
	PolkitAgentSession *session;
	gchar *action_id;
    gchar *cookie;
    gchar *message;
    GCancellable* cancellable;
	GTask* task;
	GList *identities;
    GError *error;


    GPid cmd_pid;
    int write_channel_fd;
    int read_channel_fd;
    guint read_channel_watcher;

    GIOChannel * write_channel;
    GIOChannel * read_channel;
    GString * buffer;
    GString * active_line;

    JsonParser *parser;
    JsonObject *root;

};

static void on_cancelled(GCancellable* cancellable, AuthDlgData* d);
static void init_session(AuthDlgData *d);


void blocks_mode_private_data_write_to_channel ( AuthDlgData *data, const char * format_result){
        GIOChannel * write_channel = data->write_channel;
        if(data->write_channel == NULL){
            //gets here when the script exits or there was an error loading it
            return;
        }
        log__verbose__writing_to_command_stdin(format_result);
        g_debug("sending event: %s", format_result);
        gsize bytes_witten;
        g_io_channel_write_chars(write_channel, format_result, -1, &bytes_witten, &data->error);
        g_io_channel_write_unichar(write_channel, '\n', &data->error);
        g_io_channel_flush(write_channel, &data->error);
}

static void auth_dlg_data_free(AuthDlgData *d)
{
	g_signal_handlers_disconnect_by_func((void *) d->cancellable, on_cancelled, d);

	g_object_unref(d->task);
	g_object_unref(d->session);
	g_free(d->action_id);
    g_free(d->cookie);
    g_free(d->message);
	g_list_free(d->identities);
	g_slice_free(AuthDlgData, d);
}

static gboolean on_new_input ( GIOChannel *source, GIOCondition condition, gpointer context )
{
    log__verbose__reading_command_stdout();
    AuthDlgData *data = (AuthDlgData *) context;
    GString * buffer = data->buffer;
    GString * active_line = data->active_line;

    gboolean newline = FALSE;

    GError * error = NULL;
    gunichar unichar;
    GIOStatus status;

    status = g_io_channel_read_unichar(source, &unichar, &error);

    //when there is nothing to read, status is G_IO_STATUS_AGAIN
    while(status == G_IO_STATUS_NORMAL) {
        g_string_append_unichar(buffer, unichar);
        if( unichar == '\n' ){
            if(buffer->len > 1){ //input is not an empty line
                g_debug("received new line: %s", buffer->str);
                g_string_assign(active_line, buffer->str);
                newline=TRUE;
            }
            log__verbose__received_from_command_stdout(buffer->str);
            g_string_set_size(buffer, 0);
        }
        status = g_io_channel_read_unichar(source, &unichar, &error);
    }

    if(newline){
        fprintf(stderr, "parsing line\n");

        if(! json_parser_load_from_data(data->parser,data->active_line->str,data->active_line->len,&error)){
            fprintf(stderr, "Unable to parse line: %s\n", error->message);
            g_error_free ( error );
        } else {

            data->root = json_node_get_object(json_parser_get_root(data->parser));
            const char * action = json_object_get_string_member_or_else(data->root, "action", NULL);
            if(action == NULL){
                fprintf(stderr, "no action defined, ignored");
            } else switch (accepted_action_value_of_str(action)) {
                case AcceptedAction_CANCEL: {
                                    fprintf(stderr, "action cancel");

                    g_cancellable_cancel(data->cancellable);
                    polkit_agent_session_cancel(data->session);
                }
                break;
                case AcceptedAction_AUTHENTICATE: {
                                                        fprintf(stderr, "action authenticate");

                    const char * password = json_object_get_string_member_or_else(data->root, "password", NULL);
                    if(password != NULL){
                        polkit_agent_session_response(data->session, password);
                    }
                }
                break;
                default:
                    fprintf(stderr, "unknown action %s \n", action);
            }
        }
    }

    return G_SOURCE_CONTINUE;
}

static void on_cancelled(GCancellable *cancellable, AuthDlgData *d)
{
	if (d->session)
		polkit_agent_session_cancel(d->session);
	else
		auth_dlg_data_free(d);
}


static void on_session_completed(PolkitAgentSession* session,
				 gboolean authorized, AuthDlgData* d)
{ 
      bool canceled = g_cancellable_is_cancelled(d->cancellable);
        log__verbose__polkit_session_completed(authorized, canceled);

    if(authorized){
        blocks_mode_private_data_write_to_channel(d, R"""({"action": "authorization response", "authorized": true })""");
    }
    if (authorized || canceled) {
	//	gtk_label_set_text(GTK_LABEL(d->status), NULL);
		g_task_return_pointer(d->task, NULL, NULL);
		auth_dlg_data_free(d);
		return;
	}
	g_object_unref(d->session);
	d->session = NULL;
    blocks_mode_private_data_write_to_channel(d, R"""({"action": "authorization response", "authorized": false })""");
    init_session(d);

}

static void on_session_request(PolkitAgentSession* session, gchar *req,
			       gboolean visibility, AuthDlgData *d)
{

	log__verbose__polkit_session_request(req, visibility);
    gchar * write_message;
    asprintf(&write_message, R"""({"action": "request password", "prompt":"%s", "message": "%s" })""", req, d->message);
    blocks_mode_private_data_write_to_channel(d, write_message);
    free(write_message);

	//g_debug("Request: %s\nVisibility: %i\n", req, visibility);
	//gtk_label_set_text(GTK_LABEL(d->entry_label), req);
	//gtk_entry_set_visibility(GTK_ENTRY(d->entry), visibility);
}

static void on_session_show_error(PolkitAgentSession *session, gchar *text,
				  AuthDlgData *d)
{

      log__verbose__polkit_session_show_error(text);
}

static void on_session_show_info(PolkitAgentSession *session, gchar *text,
				  AuthDlgData *d)
{
  log__verbose__polkit_session_show_info(text);
}





static void init_session(AuthDlgData *d){
  if (d->session) {
          g_signal_handlers_disconnect_matched(d->session, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, d);
          polkit_agent_session_cancel(d->session);
          g_object_unref(d->session);
  }

  PolkitIdentity *id = (PolkitIdentity *)d->identities->data;
  d->session = polkit_agent_session_new(id, d->cookie);
  g_signal_connect(d->session, "completed", G_CALLBACK(on_session_completed), d);
  g_signal_connect(d->session, "request", G_CALLBACK(on_session_request), d);
  g_signal_connect(d->session, "show-error", G_CALLBACK(on_session_show_error), d);
  g_signal_connect(d->session, "show-info", G_CALLBACK(on_session_show_info), d);
  polkit_agent_session_initiate(d->session);

}




static void initiate_authentication(PolkitAgentListener  *listener,
				    const gchar          *action_id,
				    const gchar          *message,
				    const gchar          *icon_name,
				    PolkitDetails        *details,
				    const gchar          *cookie,
				    GList                *identities,
				    GCancellable         *cancellable,
				    GAsyncReadyCallback   callback,
				    gpointer              user_data)
{

	log__verbose__init_polkit_authentication(action_id, message, icon_name, cookie);
	log__verbose__polkit_auth_identities(identities);


	AuthDlgData *d = g_slice_new0(AuthDlgData);

	char** p;

	for(p = polkit_details_get_keys(details); *p; ++p)
		g_debug("initiate_authentication: %s: %s", *p, polkit_details_lookup(details, *p));

	d->task = g_task_new(listener, cancellable, callback, user_data);
	d->cancellable = cancellable;
    d->action_id = g_strdup(action_id);
    d->message = g_strdup(message);
    d->cookie = g_strdup(cookie);
	d->identities = g_list_copy(identities);
    d->buffer = g_string_sized_new (1024);
    d->active_line = g_string_sized_new (1024);
    d->parser = json_parser_new ();
    init_session(d);


	GError *error = NULL;
	int cmd_input_fd;
	int cmd_output_fd;


    char **cmd_argv = NULL;

    if ( !g_shell_parse_argv ( app__get_cmd_line(), NULL, &cmd_argv, &error ) ){
        fprintf(stderr, "Unable to parse cmdline options: %s\n", error->message);
        g_error_free ( error ); 
    }


    if ( ! g_spawn_async_with_pipes ( NULL, cmd_argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, &(d->cmd_pid), &(cmd_input_fd), &(cmd_output_fd), NULL, &error)) {
    
        show_error_message_format("%s", error->message);
        g_cancellable_cancel(d->cancellable);
        polkit_agent_session_cancel(d->session);
    } else {
        d->read_channel_fd = cmd_output_fd;
        d->write_channel_fd = cmd_input_fd;

        int retval = fcntl( d->read_channel_fd, F_SETFL, fcntl(d->read_channel_fd, F_GETFL) | O_NONBLOCK);
        if (retval != 0){

            fprintf(stderr,"Error setting non block on output pipe\n");
            kill(d->cmd_pid, SIGTERM);
            exit(1);
        }

        d->read_channel = g_io_channel_unix_new(d->read_channel_fd);
        d->write_channel = g_io_channel_unix_new(d->write_channel_fd);
        d->read_channel_watcher = g_io_add_watch(d->read_channel, G_IO_IN, on_new_input, d);
    }
    g_strfreev(cmd_argv);




        //pd->read_channel_fd = cmd_output_fd;
        //pd->write_channel_fd = cmd_input_fd;

        //int retval = fcntl( pd->read_channel_fd, F_SETFL, fcntl(pd->read_channel_fd, F_GETFL) | O_NONBLOCK);
        //if (retval != 0){

       //     fprintf(stderr,"Error setting non block on output pipe\n");
        //    kill(pd->cmd_pid, SIGTERM);
        //    exit(1);

        //pd->read_channel = g_io_channel_unix_new(pd->read_channel_fd);
        //pd->write_channel = g_io_channel_unix_new(pd->write_channel_fd);
        //g_child_watch_add (pd->cmd_pid, on_child_status, sw);


        //polkit_agent_session_response(d->session, "teste");



}

static gboolean initiate_authentication_finish(PolkitAgentListener *listener,
				 GAsyncResult *res, GError **error)
{
	log__verbose__finish_polkit_authentication();
	return !g_task_propagate_boolean(G_TASK(res), error);
}

static void cmd_pk_agent_polkit_listener_finalize(GObject *object)
{
	log__verbose__finalize_polkit_listener();
	g_return_if_fail(object != NULL);
	g_return_if_fail(CMD_PK_AGENT_IS_POLKIT_LISTENER(object));
	G_OBJECT_CLASS(cmd_pk_agent_polkit_listener_parent_class)->finalize(object);
}

static void cmd_pk_agent_polkit_listener_class_init(CmdPkAgentPolkitListenerClass *klass)
{
	log__verbose__init_polkit_listener();
	GObjectClass *g_object_class;
	PolkitAgentListenerClass* pkal_class;
	g_object_class = G_OBJECT_CLASS(klass);
	g_object_class->finalize = cmd_pk_agent_polkit_listener_finalize;

	pkal_class = POLKIT_AGENT_LISTENER_CLASS(klass);
	pkal_class->initiate_authentication = initiate_authentication;
	pkal_class->initiate_authentication_finish = initiate_authentication_finish;
}

static void cmd_pk_agent_polkit_listener_init(CmdPkAgentPolkitListener *self)
{
}

PolkitAgentListener* cmd_pk_agent_polkit_listener_new(void)
{
	return g_object_new(CMD_PK_AGENT_TYPE_POLKIT_LISTENER, NULL);
}


