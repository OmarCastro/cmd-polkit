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
#include "../src/accepted-actions.enum.h"
#include "../src/json-glib.extension.h"
#include "error-message.mock.h"
#include "polkit-auth-handler.service.mock.h"


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


static void test_set_up (Fixture *fixture, gconstpointer user_data){
	reset_logs();
	setup_gtk_mock();
	reset_lazy_init_gtk();
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
	PolkitIdentity *  netgroup = polkit_unix_netgroup_new("testgroup");
	GList *list = NULL;
	list = g_list_append(list, user);
	list = g_list_append(list, group);
	list = g_list_append(list, netgroup);
	log__verbose__polkit_auth_identities(list);
	g_assert_cmpstr(get_stdout()->str, ==, "\
Vrbos:test_log_polkit_auth_identities:Polkit identities\n\
Vrbos:test_log_polkit_auth_identities:└- {\"type\":\"user\",\"name\":\"root\",\"id\":0,\"group id\":0}\n\
Vrbos:test_log_polkit_auth_identities:└- {\"type\":\"group\",\"name\":\"root\",\"id\":0}\n\
Vrbos:test_log_polkit_auth_identities:└- {\"type\":\"other\",\"value\":\"unix-netgroup:testgroup\"}\n\
");
	g_list_free_full(list, g_object_unref);
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
	g_list_free(invalidTypeObject);
}



static void test_log_empty_polkit_details (Fixture *fixture, gconstpointer user_data) {
	log__verbose();
	PolkitDetails* details = polkit_details_new();
	log__verbose__polkit_auth_details(details);
	g_assert_cmpstr(get_stdout()->str, ==, "\
Vrbos:test_log_empty_polkit_details:Polkit details\n\
Vrbos:test_log_empty_polkit_details:└─ (empty)\n\
");
	g_object_unref(details);
}

static void test_log_non_empty_polkit_details (Fixture *fixture, gconstpointer user_data) {
	log__verbose();
	PolkitDetails* details = polkit_details_new();
	polkit_details_insert(details, "key 1", "value");
	polkit_details_insert(details, "lorem", "ipsum");
	log__verbose__polkit_auth_details(details);
	g_assert_cmpstr(get_stdout()->str, ==, "\
Vrbos:test_log_non_empty_polkit_details:Polkit details\n\
Vrbos:test_log_non_empty_polkit_details:└─ key 1: value\n\
Vrbos:test_log_non_empty_polkit_details:└─ lorem: ipsum\n\
");
	g_object_unref(details);
}

static void test_accepted_action_value_of_str_returns_expected_values_on_valid_actions (Fixture *fixture, gconstpointer user_data) {
	g_assert_true(accepted_action_value_of_str("cancel") == AcceptedAction_CANCEL);
	g_assert_true(accepted_action_value_of_str("authenticate") == AcceptedAction_AUTHENTICATE);
}

static void test_accepted_action_value_of_str_returns_unknown_on_invalid_actions (Fixture *fixture, gconstpointer user_data) {
	g_assert_true(accepted_action_value_of_str("unknown") == AcceptedAction_UNKNOWN);
	g_assert_true(accepted_action_value_of_str(NULL) == AcceptedAction_UNKNOWN);
}

static void test_accepted_action_value_of_str_is_case_sensitive (Fixture *fixture, gconstpointer user_data) {
	g_assert_true(accepted_action_value_of_str("cancel") == AcceptedAction_CANCEL);
	g_assert_true(accepted_action_value_of_str("Cancel") == AcceptedAction_UNKNOWN);
	g_assert_true(accepted_action_value_of_str("CANCEL") == AcceptedAction_UNKNOWN);
	g_assert_true(accepted_action_value_of_str("cANCEL") == AcceptedAction_UNKNOWN);
	g_assert_true(accepted_action_value_of_str("authenticate") == AcceptedAction_AUTHENTICATE);
	g_assert_true(accepted_action_value_of_str("Authenticate") == AcceptedAction_UNKNOWN);
	g_assert_true(accepted_action_value_of_str("aUTHENTICATE") == AcceptedAction_UNKNOWN);
	g_assert_true(accepted_action_value_of_str("AUTHENTICATE") == AcceptedAction_UNKNOWN);
}

static void test_json_node_get_string_or_else (Fixture *fixture, gconstpointer user_data) {
	const gchar* text = "{\
		\"true\":true,\
		\"false\": false,\
		\"number\": 1,\
		\"string\": \"string\",\
		\"null\": null\
		}";
	GError *error = NULL;
	g_autoptr(JsonNode) root = json_from_string(text, &error);
	g_assert_no_error(error);
	g_assert_true(json_node_get_value_type(root) == JSON_TYPE_OBJECT);
	JsonObject * node = json_node_get_object(root);


	g_assert_cmpstr(json_node_get_string_or_else( json_object_get_member(node, "true") , "test") ,==, "test");
	g_assert_cmpstr(json_node_get_string_or_else( json_object_get_member(node, "false") , "test") ,==, "test");
	g_assert_cmpstr(json_node_get_string_or_else( json_object_get_member(node, "number") , "test") ,==, "test");
	g_assert_cmpstr(json_node_get_string_or_else( json_object_get_member(node, "string") , "test") ,==, "string");
	g_assert_cmpstr(json_node_get_string_or_else( json_object_get_member(node, "null") , "test") ,==, "test");
}

static void test_json_node_get_string_member_or_else (Fixture *fixture, gconstpointer user_data) {
	const gchar* text = "{\
		\"true\":true,\
		\"false\": false,\
		\"number\": 1,\
		\"string\": \"string\",\
		\"null\": null\
		}";
	GError *error = NULL;
	g_autoptr(JsonNode) root = json_from_string(text, &error);
	g_assert_no_error(error);
	g_assert_true(json_node_get_value_type(root) == JSON_TYPE_OBJECT);
	JsonObject * node = json_node_get_object(root);


	g_assert_cmpstr(json_object_get_string_member_or_else( node, "true", "test") ,==, "test");
	g_assert_cmpstr(json_object_get_string_member_or_else( node, "false" , "test") ,==, "test");
	g_assert_cmpstr(json_object_get_string_member_or_else( node, "number" , "test") ,==, "test");
	g_assert_cmpstr(json_object_get_string_member_or_else( node, "string" , "test") ,==, "string");
	g_assert_cmpstr(json_object_get_string_member_or_else( node, "null" , "test") ,==, "test");
}

static void test_error_message_dialog_inits_once (Fixture *fixture, gconstpointer user_data) {
	lazy_init_gtk();
	lazy_init_gtk();
	g_assert_true(called_times_gtk_init() == 1);
}

static void test_error_message_dialog_runs_gtk_dialog (Fixture *fixture, gconstpointer user_data) {
	show_error_message_format("hello %s", "world");
	lazy_init_gtk(); 
	lazy_init_gtk();
	g_assert_true(called_times_gtk_init() == 1);
}

static void test_polkit_auth_handler_CmdPkAgentPolkitListenerClass_init (Fixture *fixture, gconstpointer user_data) {
	PolkitAgentListener* listener = cmd_pk_agent_polkit_listener_new();
	g_object_unref(listener);
	// if there is no error, this passes
}



#define test(path, func)   g_test_add (path, Fixture, NULL, test_set_up, func, test_tear_down);

int main (int argc, char *argv[]) {

    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

	#define test(path, func)   g_test_add (path, Fixture, NULL, test_set_up, func, test_tear_down);

    // Define the tests.
    test ("/ request messages / all request messages are single line", test_all_request_messages_are_single_line);
    test ("/ request messages / request message request password is escaped correctly", test_request_message_request_password_is_escaped_correctly);
    test ("/ logger / default level logs failure and normal logs", test_default_logs);
	test ("/ logger / silenced level logs nothing", test_silenced_logs);
	test ("/ logger / polkit authentication identity information is logged as json", test_log_polkit_auth_identities);
	test ("/ logger / invalid polkit authentication identity is still logged as json, to help identify the cause of a failure", test_log_invalid_polkit_auth_identities);
	test ("/ logger / empty policy kit details is shown as empty", test_log_empty_polkit_details);
	test ("/ logger / non-empty policy kit details is shown with key: value pairs", test_log_non_empty_polkit_details);
	test ("/ accepted actions / accepted_action_value_of_str returns expected value on valid action", test_accepted_action_value_of_str_returns_expected_values_on_valid_actions);
	test ("/ accepted actions / accepted_action_value_of_str returns UNKNOWN on invalid action", test_accepted_action_value_of_str_returns_unknown_on_invalid_actions);
	test ("/ accepted actions / accepted_action_value_of_str is case sensitive", test_accepted_action_value_of_str_is_case_sensitive);
	test ("/ json glib extensions / json_node_get_string_or_else returns string value or else value if not a string", test_json_node_get_string_or_else);
	test ("/ json glib extensions / json_object_get_string_member_or_else returns string value or else value if not a string", test_json_node_get_string_member_or_else);
	test ("/ error message dialog / lazy gtk init inits gtk once", test_error_message_dialog_inits_once);
	test ("/ error message dialog / show message runs gtk dialog", test_error_message_dialog_runs_gtk_dialog);
	test ("/ polkit auth handler / CmdPkAgentPolkitListenerClass initialization", test_polkit_auth_handler_CmdPkAgentPolkitListenerClass_init);

	#undef test
	return g_test_run ();
}