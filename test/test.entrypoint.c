#include <stdbool.h>
#include <glib.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include "../src/app.h"
#include <polkit/polkit.h>
#include "src/logger.h"
#include "logger.mock.h"
#include "../src/request-messages.h"


typedef struct {
  int obj;
} Fixture;

// mock
const char*  app__get_cmd_line(){
  return "command test";
}


static void test_set_up (Fixture *fixture, gconstpointer user_data){
	reset_logs();
}

static void test_tear_down (Fixture *fixture, gconstpointer user_data){
}


gboolean has_no_newlines(const gchar *str){
	const int len = strlen(str);
	for(int i = 0; i < len; ++i){
		if(str[i] == '\n'){
			return false;
		}
	}
	return true;
}

static void test_all_request_messages_are_single_line (Fixture *fixture, gconstpointer user_data) {
	// failsafe check
	g_assert_false(has_no_newlines("lorem\nipsum"));

	g_autofree const gchar* authorized_message = request_message_authorization_authorized();
	g_assert_true(has_no_newlines(authorized_message));

	g_autofree const gchar* unauthorized_message = request_message_authorization_not_authorized();
	g_assert_true(has_no_newlines(unauthorized_message));

	g_autofree const gchar* request_pass_message = request_message_request_password("\n", "\n");
	g_assert_true(has_no_newlines(request_pass_message));
}

static void test_request_message_request_password_is_escaped_correctly (Fixture *fixture, gconstpointer user_data) {
	g_autofree const gchar* request_pass_message = request_message_request_password("\n\"", "\n\"");
	g_assert_cmpstr(request_pass_message, ==, "{\"action\":\"request password\",\"prompt\":\"\\n\\\"\",\"message\":\"\\n\\\"\"}");
}

static void test_default_logs (Fixture *fixture, gconstpointer user_data) {
	log__fail_cmdline__command_required();
	log__fail_cmdline__either_parallel_or_series();
	log__fail_cmdline__parallel_or_series_required();
	log__verbose__cmd_and_mode();
	g_assert_cmpstr(get_stderr()->str, ==, "\
Error: command argument is required\n\
Error: Only parallel or serial must be selected, not both\n\
Error: Parallel or serial option are required\n");
}

static void test_silenced_logs (Fixture *fixture, gconstpointer user_data) {
	log__silence();
	log__fail_cmdline__command_required();
	log__fail_cmdline__either_parallel_or_series();
	log__fail_cmdline__parallel_or_series_required();
	log__verbose__cmd_and_mode();
	g_assert_cmpstr(get_stderr()->str, ==, "");
}


static void test_log_polkit_auth_identities (Fixture *fixture, gconstpointer user_data) {
	log__verbose();
	PolkitIdentity * user = polkit_unix_user_new(0);
	PolkitIdentity * group = polkit_unix_group_new(0);
	GList *list = NULL;
	list = g_list_append(list, user);
	list = g_list_append(list, group);
	log__verbose__polkit_auth_identities(list);
	g_assert_cmpstr(get_stdout()->str, ==, "\
Vrbos:test_log_polkit_auth_identities:Polkit identities\n\
Vrbos:test_log_polkit_auth_identities:└- {\"type\":\"user\",\"name\":\"root\",\"id\":0,\"group id\":0}\n\
Vrbos:test_log_polkit_auth_identities:└- {\"type\":\"group\",\"name\":\"root\",\"id\":0}\n\
");
	g_list_free(list);
}

static void test_log_invalid_polkit_auth_identities (Fixture *fixture, gconstpointer user_data) {
	log__verbose();
	GList *list = NULL;
	GList *invalidTypeObject = g_list_alloc();
	list = g_list_append(list, NULL);
	list = g_list_append(list, invalidTypeObject);
	log__verbose__polkit_auth_identities(list);
	g_assert_cmpstr(get_stdout()->str, ==, "\
Vrbos:test_log_invalid_polkit_auth_identities:Polkit identities\n\
Vrbos:test_log_invalid_polkit_auth_identities:└- {\"type\":\"error\",\"error\":\"identity is null\"}\n\
Vrbos:test_log_invalid_polkit_auth_identities:└- {\"type\":\"error\",\"error\":\"invalid type: not a polkit identity\"}\n\
");
	g_list_free(list);
}

int main (int argc, char *argv[]) {

    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

    // Define the tests.
    g_test_add ("/ request messages / all request messages are single line", Fixture, NULL, test_set_up, test_all_request_messages_are_single_line, test_tear_down);
    g_test_add ("/ request messages / request message request password is escaped correctly", Fixture, NULL, test_set_up, test_request_message_request_password_is_escaped_correctly, test_tear_down);
    g_test_add ("/ logger / default level logs failure and normal logs", Fixture, NULL, test_set_up, test_default_logs, test_tear_down);
	g_test_add ("/ logger / silenced level logs nothing", Fixture, NULL, test_set_up, test_silenced_logs, test_tear_down);
	g_test_add ("/ logger / polkit authentication identity information is logged as json", Fixture, NULL, test_set_up, test_log_polkit_auth_identities, test_tear_down);
	g_test_add ("/ logger / invalid polkit authentication identity is still logged as json, to help identify the cause of a failure", Fixture, NULL, test_set_up, test_log_invalid_polkit_auth_identities, test_tear_down);

  return g_test_run ();
}