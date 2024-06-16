// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
#include <stdbool.h>
#define _GNU_SOURCE
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <string.h>
#include <gmodule.h>
#include <time.h>
#include <fcntl.h>
#include "logger.h"
#include "app.h"
#include "json-glib.extension.h"
#include "accepted-actions.enum.h"
#include "error-message.dialog.h"
#include "polkit-auth-handler.service.h"
#include "request-messages.h"

G_DEFINE_TYPE(CmdPkAgentPolkitListener, cmd_pk_agent_polkit_listener, POLKIT_AGENT_TYPE_LISTENER)

typedef enum {
    IN_QUEUE,
    AUTHENTICATING,
    CANCELED,
    AUTHORIZED
} AuthDlgDataStatus;

typedef struct _AuthDlgData AuthDlgData;
struct _AuthDlgData {
	PolkitAgentSession *session;
    PolkitActionDescription* action_description;
	gchar *action_id;
    gchar *cookie;
    gchar *message;
	GTask* task;
	GList *identities;
    GError *error;

    AuthDlgDataStatus status;

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

/** 
 * Authentication queue to be used in serial mode
 * The an authentication goes to the queur if there is one authentication currently being handled
 */
GAsyncQueue * serial_mode_queue = NULL;
AuthDlgData * serial_mode_current_authentication = NULL;

bool serie_mode_is_queue_empty(){
    return serial_mode_queue == NULL || g_async_queue_length(serial_mode_queue) <= 0;
}

void serie_mode_push_auth_to_queue(AuthDlgData *d){
    if(serial_mode_queue == NULL){
        serial_mode_queue = g_async_queue_new();
    }
    g_async_queue_push(serial_mode_queue, d);
}

AuthDlgData* serie_mode_pop_auth_from_queue(){
    if(serial_mode_queue == NULL){
        serial_mode_queue = g_async_queue_new();
    }
    return (AuthDlgData *) g_async_queue_pop(serial_mode_queue);
}

static void build_session(AuthDlgData *d);
static void spawn_command_for_authentication(AuthDlgData *d);

void auth_dialog_data_write_to_channel ( AuthDlgData *data, const char * message){
        GIOChannel * write_channel = data->write_channel;
        if(data->write_channel == NULL){
            //gets here when the script exits or there was an error loading it
            return;
        }
        log__verbose__writing_to_command_stdin(message);
        gsize bytes_witten;
        g_io_channel_write_chars(write_channel, message, -1, &bytes_witten, &data->error);
        g_io_channel_write_unichar(write_channel, '\n', &data->error);
        g_io_channel_flush(write_channel, &data->error);
}

static void auth_dlg_data_run_and_free_task(AuthDlgData *d){
    GTask *task = d->task;
    if(task != NULL){
		g_task_return_boolean(task, true);
        g_object_unref(task);
        d->task = NULL;
    }
}

static void auth_dlg_data_free(AuthDlgData *d)
{
    GError* error = NULL;

    auth_dlg_data_run_and_free_task(d);
	g_object_unref(d->session);
	g_free(d->action_id);
    g_free(d->cookie);
    g_free(d->message);
	g_list_free(d->identities);
    g_source_remove (d->read_channel_watcher);
    g_io_channel_shutdown(d->write_channel, TRUE, &error);
    if(error){
        fprintf(stderr, "error closing write channel of pid %d: %s\n", d->cmd_pid, error->message);
        g_error_free ( error );
    }
    g_io_channel_unref(d->write_channel);
    g_io_channel_shutdown(d->read_channel, FALSE, &error);
    if(error){
        fprintf(stderr, "error closing read channel of pid %d: %s\n", d->cmd_pid, error->message);
        g_error_free ( error );
    }
    if(d->action_description != NULL){
        g_object_unref(d->action_description);
    }
    g_io_channel_unref(d->read_channel);
    g_string_free(d->active_line, true);
    g_string_free(d->buffer, true);
    g_object_unref(d->parser);
	g_slice_free(AuthDlgData, d);
}

static gboolean on_new_input ( GIOChannel *source, [[maybe_unused]] GIOCondition condition, gpointer context )
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
                    data->status = CANCELED;
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

static void on_session_completed([[maybe_unused]] PolkitAgentSession* session, gboolean authorized, AuthDlgData* d)
{ 
    bool canceled = d->status == CANCELED;
    log__verbose__polkit_session_completed(authorized, canceled);

    if(authorized){
        d->status = AUTHORIZED;
        g_autofree const char* message = request_message_authorization_authorized();
        auth_dialog_data_write_to_channel(d, message);
    }
    if (authorized || canceled) {
		auth_dlg_data_run_and_free_task(d);
		auth_dlg_data_free(d);
        if(app__get_auth_handling_mode() == AuthHandlingMode_SERIE){
            if(serie_mode_is_queue_empty()){
                serial_mode_current_authentication = NULL;
            } else {
                AuthDlgData* data = serie_mode_pop_auth_from_queue();
                data->status = AUTHENTICATING;
                serial_mode_current_authentication = data;
                spawn_command_for_authentication(data);
                polkit_agent_session_initiate(data->session);
            }
        }

		return;
	}
	g_object_unref(d->session);
	d->session = NULL;
    g_autofree const char* message = request_message_authorization_not_authorized();
    auth_dialog_data_write_to_channel(d, message);
    build_session(d);
    polkit_agent_session_initiate(d->session);

}

static void on_session_request([[maybe_unused]] PolkitAgentSession* session, gchar *req, gboolean visibility, AuthDlgData *d)
{
	log__verbose__polkit_session_request(req, visibility);
    g_autofree const char *write_message = request_message_request_password(req, d->message, d->action_description);
    auth_dialog_data_write_to_channel(d, write_message);
}

static void on_session_show_error([[maybe_unused]] PolkitAgentSession* session, gchar *text, [[maybe_unused]] AuthDlgData* d)
{

    log__verbose__polkit_session_show_error(text);
}

static void on_session_show_info([[maybe_unused]] PolkitAgentSession *session, gchar *text, [[maybe_unused]] AuthDlgData* d)
{
    log__verbose__polkit_session_show_info(text);
}

static void build_session(AuthDlgData *d){
  if (G_UNLIKELY(d->session)) {
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

}

static void spawn_command_for_authentication(AuthDlgData *d){
	GError *error = NULL;
	int cmd_input_fd;
	int cmd_output_fd;

    char ** const cmd_argv = app__get_cmd_line_argv();

    if ( ! g_spawn_async_with_pipes ( NULL, cmd_argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &(d->cmd_pid), &(cmd_input_fd), &(cmd_output_fd), NULL, &error)) {
        show_error_message_format("%s", error->message);
        polkit_agent_session_cancel(d->session);
        return;
    } 
    d->read_channel_fd = cmd_output_fd;
    d->write_channel_fd = cmd_input_fd;

    int retval = fcntl( d->read_channel_fd, F_SETFL, fcntl(d->read_channel_fd, F_GETFL) | O_NONBLOCK);
    if (retval != 0){
        fprintf(stderr,"Error setting non block on output pipe\n");
        kill(d->cmd_pid, SIGTERM);
        polkit_agent_session_cancel(d->session);
        return;
    }

    d->read_channel = g_io_channel_unix_new(d->read_channel_fd);
    d->write_channel = g_io_channel_unix_new(d->write_channel_fd);
    d->read_channel_watcher = g_io_add_watch(d->read_channel, G_IO_IN, on_new_input, d);
}


/**
 * Authentication request handler of PolkitAgentListener.
 *
 */
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
	log__verbose__polkit_auth_details(details);

	AuthDlgData *d = g_slice_new0(AuthDlgData);

    GError *error = NULL;
    PolkitAuthority* authority = polkit_authority_get_sync(NULL, &error);
    if(error == NULL){
        GList* actions = polkit_authority_enumerate_actions_sync (authority,NULL,&error);
        if(error == NULL){
            for(GList *elem = actions; elem; elem = elem->next) {
                PolkitActionDescription* action_description = elem->data;
                if(d->action_description != NULL){
                    // continue to g_object_unref the remaining elements on the list, as they are required
                    // before freeing the `actions` GList 
                    g_object_unref(action_description);
                    continue; 
                }

                const gchar * action_description_action_id = polkit_action_description_get_action_id(action_description);
                if(strcmp(action_description_action_id, action_id) == 0){
                    log__verbose__polkit_action_description(action_description);
                    g_object_ref(action_description);
                    d->action_description = action_description;
                }

                g_object_unref(action_description);
            }
            g_list_free(actions);
        }
        g_object_unref(authority);
    }

	d->task = g_task_new(listener, cancellable, callback, user_data);
    d->action_id = g_strdup(action_id);
    d->message = g_strdup(message);
    d->cookie = g_strdup(cookie);
	d->identities = g_list_copy(identities);
    d->buffer = g_string_sized_new (1024);
    d->active_line = g_string_sized_new (1024);
    d->parser = json_parser_new ();
    build_session(d);
    if(app__get_auth_handling_mode() == AuthHandlingMode_PARALLEL){
        spawn_command_for_authentication(d);
        polkit_agent_session_initiate(d->session);
    } else if(serial_mode_current_authentication != NULL){
        d->status = IN_QUEUE;
        serie_mode_push_auth_to_queue(d);
    } else {
        d->status = AUTHENTICATING;
        serial_mode_current_authentication = d;
        spawn_command_for_authentication(d);
        polkit_agent_session_initiate(d->session);
    }
}

static gboolean initiate_authentication_finish(
    [[maybe_unused]] PolkitAgentListener *listener,
    GAsyncResult *res,
    GError **error)
{
	log__verbose__finish_polkit_authentication();
	return g_task_propagate_boolean(G_TASK(res), error);
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

static void cmd_pk_agent_polkit_listener_init([[maybe_unused]] CmdPkAgentPolkitListener *self)
{
}

PolkitAgentListener* cmd_pk_agent_polkit_listener_new(void)
{
	return g_object_new(CMD_PK_AGENT_TYPE_POLKIT_LISTENER, NULL);
}


