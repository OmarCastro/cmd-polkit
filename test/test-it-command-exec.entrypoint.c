#include <stdbool.h>
#include <glib.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include "../src/app.h"
#include <polkit/polkit.h>
#include "error-message.mock.h"
#include "polkit-auth-handler.service.mock.h"

gboolean quit = FALSE;

typedef struct {
  int obj;
} Fixture;


// mock
const char*  app__get_cmd_line(){
  return "bash ./assets/test_response_command.sh";
}

Application app_get(){
  return (Application){
      .argc = 0 ,
      .argv = NULL ,
      .command_line = app__get_cmd_line() ,
      .handling_mode = AuthHandlingMode_PARALLEL
  };
}

void quitloop(gpointer data){
	printf("ok 1 / polkit auth handler / CmdPkAgentPolkitListener initiate_authentication procedure testing \n");
	quit = true;
}

void prepare_to_exit(){
	g_idle_add_once(quitloop, NULL);
}

static void test_polkit_auth_handler_authentication_aux (gpointer main_loop) {
	PolkitAgentListener* listener = cmd_pk_agent_polkit_listener_new();
	const gchar *action_id = g_strdup("org.freedesktop.policykit.exec");
	const gchar *message = g_strdup("Authentication is needed to run `/usr/bin/echo 1' as the super user");
	const gchar *icon_name = g_strdup("");
	const gchar *cookie = g_strdup("3-97423289449bd6d0c3915fb1308b9814-1-a305f93fec6edd353d6d1845e7fcf1b2");
	g_autoptr(PolkitDetails) details = polkit_details_new();
	PolkitIdentity * user = polkit_unix_user_new(1000);
	GList *identities = NULL;
	identities = g_list_append(identities, user);

	POLKIT_AGENT_LISTENER_GET_CLASS(listener)->initiate_authentication(
		listener,action_id,message,icon_name,details,cookie,identities,NULL,prepare_to_exit,NULL
	);
}


static void test_polkit_auth_handler_authentication (Fixture *fixture, gconstpointer user_data) {
	GMainLoop *loop = g_main_loop_new (NULL, FALSE);
	g_idle_add_once(test_polkit_auth_handler_authentication_aux, loop);
	GMainContext *context = g_main_context_default ();

	/* Run the GLib event loop. */
	while (!quit) {
		g_main_context_iteration (context, TRUE);
	}
	g_main_loop_unref(loop);
}



#define test(path, func)   g_test_add (path, Fixture, NULL, test_set_up, func, test_tear_down);

int main (int argc, char *argv[]) {

    setlocale (LC_ALL, "");

	printf("TAP version 13\n");
	printf("1..1\n");


    test_polkit_auth_handler_authentication(NULL, NULL);
}