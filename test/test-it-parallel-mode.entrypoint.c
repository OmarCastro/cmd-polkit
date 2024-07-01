#include <stdbool.h>
#include <glib.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include "app.mock.h"
#include <polkit/polkit.h>
#include "error-message.mock.h"
#include "polkit-auth-handler.service.mock.h"
#include "src/app.h"
#include "test/app.mock.h"


typedef struct {
  PolkitAgentListener* listener;
  GList * identities;
  PolkitDetails *details;
  GMainLoop *loop;
} Fixture;

char *current_cmd_line = "bash ./assets/test_response_command.sh";


const int test_argc = 4;
char * test_argv[] = {
	"cmd-polkit-agent",
	"-p", 
	"-c", 
	"bash ./assets/test_response_parallel.sh", 
	NULL
};

static int quitloop(gpointer fixture_ptr){
	Fixture *fixture = fixture_ptr;
	g_main_loop_quit(fixture->loop);
	return G_SOURCE_REMOVE;

}

static void finish_autentication([[maybe_unused]] GObject *obj, GAsyncResult * result, gpointer fixture_ptr){
	Fixture *fixture = fixture_ptr;
	PolkitAgentListener *listener = fixture->listener;
	GError *error = NULL;
	POLKIT_AGENT_LISTENER_GET_CLASS (listener)->initiate_authentication_finish (listener, result, &error);
}

static void finish_autentication_chek_serial_exit([[maybe_unused]] GObject *obj, GAsyncResult * result, gpointer fixture_ptr){
	Fixture *fixture = fixture_ptr;
	PolkitAgentListener *listener = fixture->listener;
	GError *error = NULL;
	POLKIT_AGENT_LISTENER_GET_CLASS (listener)->initiate_authentication_finish (listener, result, &error);
	gchar * contents = NULL;
	if(g_file_get_contents("test-parallel.timings.txt", &contents, NULL, NULL)){
		GFile* file = g_file_new_for_path ("test-parallel.timings.txt");
		g_file_delete(file, NULL, NULL);
		g_object_unref(file);
		g_assert_cmpstr(contents, ==, "\
start\n\
start\n\
end\n\
end\n\
");
	}
	g_free(contents);
	g_idle_add(quitloop, fixture);
}

static int test_polkit_auth_handler_authentication_aux_serial (gpointer fixture_ptr) {
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
		finish_autentication,
		fixture
	);

		POLKIT_AGENT_LISTENER_GET_CLASS(fixture->listener)->initiate_authentication(
		fixture->listener,
		action_id,message,
		icon_name,
		fixture->details,
		cookie,
		fixture->identities,
		NULL,
		finish_autentication_chek_serial_exit,
		fixture
	);


	return G_SOURCE_REMOVE;
}



static void test_polkit_auth_handler_authentication_serial_mode (Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {

	app__init(test_argc, test_argv);
	fixture->loop = g_main_loop_new (NULL, FALSE);
	g_idle_add(test_polkit_auth_handler_authentication_aux_serial, fixture);
	g_main_loop_run(fixture->loop);
}



static void test_set_up ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data){
	app__reset();
}

static void test_tear_down (Fixture *fixture, [[maybe_unused]] gconstpointer user_data){
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
	test ("/ polkit auth handler / CmdPkAgentPolkitListener initiate_authentication procedure serial mode", test_polkit_auth_handler_authentication_serial_mode);

	#undef test
	return g_test_run ();
}