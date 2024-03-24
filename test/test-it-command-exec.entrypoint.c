#include <stdbool.h>
#include <glib.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include "../src/app.h"
#include <polkit/polkit.h>
#include "error-message.mock.h"
#include "polkit-auth-handler.service.mock.h"


typedef struct {
  PolkitAgentListener* listener;
  GList * identities;
  PolkitDetails *details;
  GMainLoop *loop;
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

int quitloop(gpointer loop){
	g_main_loop_quit(loop);
	return G_SOURCE_REMOVE;

}

void prepare_to_exit(GObject *obj, GAsyncResult * result, gpointer loop){
	g_idle_add(quitloop, loop);
}

static int test_polkit_auth_handler_authentication_aux (gpointer fixture_ptr) {
	Fixture *fixture = fixture_ptr;
	fixture->listener = cmd_pk_agent_polkit_listener_new();
	const gchar *action_id = "org.freedesktop.policykit.exec";
	const gchar *message = "Authentication is needed to run `/usr/bin/echo 1' as the super user";
	const gchar *icon_name = "";
	const gchar *cookie = "3-97423289449bd6d0c3915fb1308b9814-1-a305f93fec6edd353d6d1845e7fcf1b2";
	fixture->details = polkit_details_new();
	PolkitIdentity *user = polkit_unix_user_new(1000);
	fixture->identities = g_list_append(fixture->identities, user);

	POLKIT_AGENT_LISTENER_GET_CLASS(fixture->listener)->initiate_authentication(
		fixture->listener,
		action_id,message,
		icon_name,
		fixture->details,
		cookie,
		fixture->identities,
		NULL,
		prepare_to_exit,
		fixture->loop
	);


	return G_SOURCE_REMOVE;
}


static void test_polkit_auth_handler_authentication (Fixture *fixture, gconstpointer user_data) {
	fixture->loop = g_main_loop_new (NULL, FALSE);
	g_idle_add(test_polkit_auth_handler_authentication_aux, fixture);
	g_main_loop_run(fixture->loop);
}


static void test_set_up (Fixture *fixture, gconstpointer user_data){

}

static void test_tear_down (Fixture *fixture, gconstpointer user_data){
	if(fixture->listener != NULL){
		g_object_unref(fixture->listener);
		fixture->listener = NULL;
	}
	if(fixture->identities != NULL){
		g_list_free_full(fixture->identities, g_object_unref);
		fixture->identities = NULL;
	}
	if(fixture->details != NULL){
		g_object_unref(fixture->details);
		fixture->details = NULL;
	}
	g_main_loop_unref(fixture->loop);

}


int main (int argc, char *argv[]) {

    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

	#define test(path, func)   g_test_add (path, Fixture, NULL, test_set_up, func, test_tear_down);

    // Define the tests.
	test ("/ polkit auth handler / CmdPkAgentPolkitListener initiate_authentication procedure testing", test_polkit_auth_handler_authentication);

	#undef test
	return g_test_run ();
}