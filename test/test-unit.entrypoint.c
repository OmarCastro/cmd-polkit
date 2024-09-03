#include <stdbool.h>
#include <glib.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include "app.mock.h"
#include <polkit/polkit.h>
#include "src/app.h"
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

const int test_argc = 4;
char * test_argv[] = {
	"cmd-polkit-agent",
	"-p", 
	"-c", 
	"bash ./assets/test_response_command.sh", 
	NULL
};


static void test_set_up ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data){
	reset_logs();
	setup_gtk_mock();
	reset_lazy_init_gtk();
	app__init(test_argc, test_argv);

}

static void test_tear_down ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data){
	app__reset();
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


#define HELP_MESSAGE "\
Usage: cmd-polkit-agent -s|--serial|-p|parallel -c|--command COMMAND\n\
Polkit agent that allows to easily customize the UI to authenticate on polkit.\n\
\n\
Runs COMMAND for each authentication request and communicates with it via JSON\n\
messages through stdin and stdout. It allows to easily create a GUI to\n\
authenticate on polkit.\n\
\n\
  -h, --help             Print help and exit\n\
  -V, --version          Print version and exit\n\
  -c, --command=COMMAND  Command to execute on authorization request\n\
  -s, --serial           handle one authorization request at a time\n\
  -p, --parallel         handle authorization in parallel\n\
  -v, --verbose          Increase program verbosity\n\
  -q, --quiet            Do not print anything\n\
      --silent           \n\
\n\
Full documentation <https://omarcastro.github.io/cmd-polkit>\n\
"

void write_to_file(const char *name, const char *content){
	FILE *file;
	file = fopen(name, "w");
	if (file == NULL)
	{
		printf("error\n");
	} else {
		fprintf(file, "%s", content);
	}
	fclose(file);
}


void append_found_polkit_action_description(GList* actions){
	const char* build_dir = getenv("G_TEST_BUILDDIR");
	if(build_dir == NULL){ return; }
	gchar* file = g_strconcat(build_dir, "/polkit-action-ids.txt", NULL);
	GString *string = g_string_sized_new(1024);
	for(GList *elem = actions; elem; elem = elem->next) {
    PolkitActionDescription* action_description = elem->data;
		const gchar * action_description_action_id = polkit_action_description_get_action_id(action_description);
		g_string_append_printf(string, "%s\n", action_description_action_id);
	}	
	write_to_file(file, string->str);

	g_free(file);

}

PolkitActionDescription * get_test_polkit_action_description(){
	static bool actions_written = false;
	gchar * action_id = "org.freedesktop.login1.halt";
	PolkitActionDescription * result = NULL;
	GError *error = NULL;
    PolkitAuthority* authority = polkit_authority_get_sync(NULL, &error);
    if(error == NULL){
        GList* actions = polkit_authority_enumerate_actions_sync (authority,NULL,&error);
        if(error == NULL){
			if(!actions_written){
				append_found_polkit_action_description(actions);
				actions_written = true;
			}
            for(GList *elem = actions; elem; elem = elem->next) {
                PolkitActionDescription* action_description = elem->data;
                const gchar * action_description_action_id = polkit_action_description_get_action_id(action_description);
                if(strcmp(action_description_action_id, action_id) == 0){
                    g_object_ref(action_description);
					result = action_description;
                }
                g_object_unref(action_description);
            }
            g_list_free(actions);
        }
        g_object_unref(authority);
    } else {
		// Report error to user, and free error
		g_assert (authority == NULL);
		fprintf (stderr, "Unable to get authority: %s\n", error->message);
		g_error_free (error);
	}
	return result;
}

static void test_all_request_messages_are_single_line ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	// failsafe check
	g_assert_false(has_no_newlines("lorem\nipsum"));

	g_autofree const gchar* authorized_message = request_message_authorization_authorized();
	g_assert_true(has_no_newlines(authorized_message));

	g_autofree const gchar* unauthorized_message = request_message_authorization_not_authorized();
	g_assert_true(has_no_newlines(unauthorized_message));

	g_autofree const gchar* request_pass_message = request_message_request_password("\n", "\n", NULL);
	g_assert_true(has_no_newlines(request_pass_message));
}

static void test_request_message_request_password_is_escaped_correctly ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	g_autofree const gchar* request_pass_message_1 = request_message_request_password("\n\"", "no \n polkit \" action", NULL);
	g_assert_cmpstr(request_pass_message_1, ==, "{\"action\":\"request password\",\"prompt\":\"\\n\\\"\",\"message\":\"no \\n polkit \\\" action\",\"polkit action\":null}");


	PolkitActionDescription* action_description = get_test_polkit_action_description();
	if(action_description) {
		g_autofree const gchar* request_pass_message_2 = request_message_request_password("\n\"", "\n\"", action_description);
		g_object_unref(action_description);
		g_assert_cmpstr(request_pass_message_2, ==, "{\"action\":\"request password\",\"prompt\":\"\\n\\\"\",\"message\":\"\\n\\\"\",\"polkit action\":{\"id\":\"org.freedesktop.login1.halt\",\"description\":\"Halt the system\",\"message\":\"Authentication is required to halt the system.\",\"vendor name\":\"The systemd Project\",\"vendor url\":\"https://systemd.io\",\"icon name\":\"\"}}");
	}
}

static void test_default_logs ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	log__fail_cmdline__command_required();
	log__fail_cmdline__either_parallel_or_series();
	log__fail_cmdline__parallel_or_series_required();
	log__verbose__cmd_and_mode();
	g_assert_cmpstr(get_stderr()->str, ==, "\
Error parsing command line: command argument is required\n\
\n\
"HELP_MESSAGE"\
Error parsing command line: only serial or parallel mode must be selected, not both\n\
\n\
"HELP_MESSAGE"\
Error parsing command line: parallel or serial mode is required\n\
\n\
"HELP_MESSAGE);
	g_assert_cmpstr(get_stdout()->str, ==, "");
}

static void test_silenced_logs ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	log__silence();
	log__fail_cmdline__command_required();
	log__fail_cmdline__either_parallel_or_series();
	log__fail_cmdline__parallel_or_series_required();
	log__verbose__cmd_and_mode();
	g_assert_cmpstr(get_stderr()->str, ==, "");
	g_assert_cmpstr(get_stdout()->str, ==, "");
}

static void test_verbose_logs ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	log__verbose();
	log__fail_cmdline__command_required();
	log__fail_cmdline__either_parallel_or_series();
	log__fail_cmdline__parallel_or_series_required();
	log__verbose__cmd_and_mode();
	log__verbose__polkit_session_show_error("test");
	log__verbose__writing_to_command_stdin("{\"content\":\"test\"}");
	log__verbose__received_from_command_stdout("{\"content\":\"test\"}");
	log__verbose__reading_command_stdout();
	log__verbose__finish_polkit_authentication();
	log__verbose__polkit_session_show_info("<Polkit info>");
	log__verbose__polkit_session_request("Password:", true);
	log__verbose__polkit_session_request("Password:", false);
	g_assert_cmpstr(get_stderr()->str, ==, "\
Error parsing command line: command argument is required\n\
\n\
"HELP_MESSAGE"\
Error parsing command line: only serial or parallel mode must be selected, not both\n\
\n\
"HELP_MESSAGE"\
Error parsing command line: parallel or serial mode is required\n\
\n\
"HELP_MESSAGE"\
");
	g_assert_cmpstr(get_stdout()->str, ==, "\
Vrbos:test_verbose_logs:COMMAND TO EXECUTE: bash ./assets/test_response_command.sh\n\
Vrbos:test_verbose_logs:AUTH HANDLING MODE: PARALLEL\n\
Vrbos:test_verbose_logs:Polkit session show error: test\n\
Vrbos:test_verbose_logs:writing to command stdin: {\"content\":\"test\"}\n\
Vrbos:test_verbose_logs:received line from command stdout: {\"content\":\"test\"}\n\
Vrbos:test_verbose_logs:reading output\n\
Vrbos:test_verbose_logs:finish Polkit authentication\n\
Vrbos:test_verbose_logs:Polkit session show info: <Polkit info>\n\
Vrbos:test_verbose_logs:Polkit session request: Password:\n\
Vrbos:test_verbose_logs:└─ visibility: yes\n\
Vrbos:test_verbose_logs:Polkit session request: Password:\n\
Vrbos:test_verbose_logs:└─ visibility: no\n\
");
}



static void test_log_verbose_init_polkit_authentication ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	log__verbose();
	const gchar *action_id = "org.freedesktop.policykit.exec";
	const gchar *message = "Authentication is needed to run `/usr/bin/echo 1' as the super user";
	const gchar *icon_name = "";
	const gchar *cookie = "3-97423289449bd6d0c3915fb1308b9814-1-a305f93fec6edd353d6d1845e7fcf1b2";

	log__verbose__init_polkit_authentication(action_id, message, icon_name, cookie);
	g_assert_cmpstr(get_stdout()->str, ==, "\
Vrbos:test_log_verbose_init_polkit_authentication:initiate Polkit authentication\n\
Vrbos:test_log_verbose_init_polkit_authentication:└─ action id : org.freedesktop.policykit.exec\n\
Vrbos:test_log_verbose_init_polkit_authentication:└─ message   : Authentication is needed to run `/usr/bin/echo 1' as the super user\n\
Vrbos:test_log_verbose_init_polkit_authentication:└─ icon name : \n\
Vrbos:test_log_verbose_init_polkit_authentication:└─ cookie    : 3-97423289449bd6d0c3915fb1308b9814-1-a305f93fec6edd353d6d1845e7fcf1b2\n\
");
}

static void test_log_polkit_auth_identities ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
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
Vrbos:test_log_polkit_auth_identities:└─ {\"type\":\"user\",\"name\":\"root\",\"id\":0,\"group id\":0}\n\
Vrbos:test_log_polkit_auth_identities:└─ {\"type\":\"group\",\"name\":\"root\",\"id\":0}\n\
Vrbos:test_log_polkit_auth_identities:└─ {\"type\":\"other\",\"value\":\"unix-netgroup:testgroup\"}\n\
");
	g_list_free_full(list, g_object_unref);
}

static void test_log_invalid_polkit_auth_identities ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	log__verbose();
	GList *list = NULL;
	GList *invalidTypeObject = g_list_alloc();
	list = g_list_append(list, NULL);
	list = g_list_append(list, invalidTypeObject);
	log__verbose__polkit_auth_identities(list);
	g_assert_cmpstr(get_stdout()->str, ==, "\
Vrbos:test_log_invalid_polkit_auth_identities:Polkit identities\n\
Vrbos:test_log_invalid_polkit_auth_identities:└─ {\"type\":\"error\",\"error\":\"identity is null\"}\n\
Vrbos:test_log_invalid_polkit_auth_identities:└─ {\"type\":\"error\",\"error\":\"invalid type: not a polkit identity\"}\n\
");
	g_list_free(list);
	g_list_free(invalidTypeObject);
}

static void test_log_polkit_action_description ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	log__verbose();
	PolkitActionDescription* action_description = get_test_polkit_action_description();
	log__verbose__polkit_action_description(action_description);
	g_object_unref(action_description);

	g_assert_cmpstr(get_stdout()->str, ==, "\
Vrbos:test_log_polkit_action_description:Polkit action description\n\
Vrbos:test_log_polkit_action_description:└─ id: org.freedesktop.login1.halt\n\
Vrbos:test_log_polkit_action_description:└─ description: Halt the system\n\
Vrbos:test_log_polkit_action_description:└─ message: Authentication is required to halt the system.\n\
Vrbos:test_log_polkit_action_description:└─ vendor name: The systemd Project\n\
Vrbos:test_log_polkit_action_description:└─ vendor url: https://systemd.io\n\
Vrbos:test_log_polkit_action_description:└─ annotations:\n\
Vrbos:test_log_polkit_action_description:   └─ org.freedesktop.policykit.imply: org.freedesktop.login1.set-wall-message\n\
");
}


static void test_log_empty_polkit_details ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	log__verbose();
	PolkitDetails* details = polkit_details_new();
	log__verbose__polkit_auth_details(details);
	g_assert_cmpstr(get_stdout()->str, ==, "\
Vrbos:test_log_empty_polkit_details:Polkit details\n\
Vrbos:test_log_empty_polkit_details:└─ (empty)\n\
");
	g_object_unref(details);
}

static void test_log_non_empty_polkit_details ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
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

static void test_log_polkit_session_completed ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	log__verbose();
	log__verbose__polkit_session_completed(false, false);
	log__verbose__polkit_session_completed(false, true);
	log__verbose__polkit_session_completed(true, false);
	log__verbose__polkit_session_completed(true, true);
	g_assert_cmpstr(get_stdout()->str, ==, "\
Vrbos:test_log_polkit_session_completed:Polkit session completed\n\
Vrbos:test_log_polkit_session_completed:└─ {\"authorized\": \"no\", \"canceled\":\"no\" })\n\
Vrbos:test_log_polkit_session_completed:Polkit session completed\n\
Vrbos:test_log_polkit_session_completed:└─ {\"authorized\": \"no\", \"canceled\":\"yes\" })\n\
Vrbos:test_log_polkit_session_completed:Polkit session completed\n\
Vrbos:test_log_polkit_session_completed:└─ {\"authorized\": \"yes\", \"canceled\":\"no\" })\n\
Vrbos:test_log_polkit_session_completed:Polkit session completed\\
nVrbos:test_log_polkit_session_completed:└─ {\"authorized\": \"yes\", \"canceled\":\"yes\" })\n\
");
}

static void test_accepted_action_value_of_str_returns_expected_values_on_valid_actions ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	g_assert_true(accepted_action_value_of_str("cancel") == AcceptedAction_CANCEL);
	g_assert_true(accepted_action_value_of_str("authenticate") == AcceptedAction_AUTHENTICATE);
}

static void test_accepted_action_value_of_str_returns_unknown_on_invalid_actions ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	g_assert_true(accepted_action_value_of_str("unknown") == AcceptedAction_UNKNOWN);
	g_assert_true(accepted_action_value_of_str(NULL) == AcceptedAction_UNKNOWN);
}

static void test_accepted_action_value_of_str_is_case_sensitive ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	g_assert_true(accepted_action_value_of_str("cancel") == AcceptedAction_CANCEL);
	g_assert_true(accepted_action_value_of_str("Cancel") == AcceptedAction_UNKNOWN);
	g_assert_true(accepted_action_value_of_str("CANCEL") == AcceptedAction_UNKNOWN);
	g_assert_true(accepted_action_value_of_str("cANCEL") == AcceptedAction_UNKNOWN);
	g_assert_true(accepted_action_value_of_str("authenticate") == AcceptedAction_AUTHENTICATE);
	g_assert_true(accepted_action_value_of_str("Authenticate") == AcceptedAction_UNKNOWN);
	g_assert_true(accepted_action_value_of_str("aUTHENTICATE") == AcceptedAction_UNKNOWN);
	g_assert_true(accepted_action_value_of_str("AUTHENTICATE") == AcceptedAction_UNKNOWN);
}

static void test_json_node_get_string_or_else ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
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

static void test_json_node_get_string_member_or_else ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
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

static void test_error_message_dialog_inits_once ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	lazy_init_gtk();
	lazy_init_gtk();
	g_assert_true(called_times_gtk_init() == 1);
}

static void test_error_message_dialog_runs_gtk_dialog ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	show_error_message_format("hello %s", "world");
	lazy_init_gtk(); 
	lazy_init_gtk();
	g_assert_true(called_times_gtk_init() == 1);
}

static void test_polkit_auth_handler_CmdPkAgentPolkitListenerClass_init ([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	PolkitAgentListener* listener = cmd_pk_agent_polkit_listener_new();
	g_object_unref(listener);
	// if there is no error, this passes
}

static void test_app_init_without_arguments_shows_help_message([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	char* argv[] = {
		"cmd-polkit-agent", 
		NULL
	};
	app__init(1, argv);
	g_assert_cmpstr(get_stderr()->str, ==, "\
Error parsing command line: command argument is required\n\
\n\
"HELP_MESSAGE);
	g_assert_cmpstr(get_stdout()->str, ==, "");
}

static void test_app_init_with_invalid_arguments_shows_help_message([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	char* argv[] = {
		"cmd-polkit-agent", 
		"--lorem", 
		"-c", 
		"bash test.sh", 
		NULL
	};
	app__init(4, argv);
	g_assert_cmpstr(get_stderr()->str, ==, "\
cmd-polkit-agent: unrecognized option `--lorem'\n\
\n\
"HELP_MESSAGE);
	g_assert_cmpstr(get_stdout()->str, ==, "");
}

static void test_app_init_without_serial_and_parallel_shows_error([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	char* argv[] = {
		"cmd-polkit-agent", 
		"-c", 
		"bash test.sh", 
		NULL
	};
	app__init(3, argv);
	g_assert_cmpstr(get_stderr()->str, ==, "\
Error parsing command line: parallel or serial mode is required\n\
\n\
"HELP_MESSAGE);
	g_assert_cmpstr(get_stdout()->str, ==, "");
}


static void test_app_init_with_both_serial_and_parallel_shows_error([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	char* argv[] = {
		"cmd-polkit-agent", 
		"-sp",
		"-c", 
		"bash test.sh", 
		NULL
	};
	app__init(4, argv);
	g_assert_cmpstr(get_stderr()->str, ==, "\
Error parsing command line: only serial or parallel mode must be selected, not both\n\
\n\
"HELP_MESSAGE);
	g_assert_cmpstr(get_stdout()->str, ==, "");
}


static void test_app_init_with_blank_command_line_shows_error([[maybe_unused]] Fixture *fixture, [[maybe_unused]] gconstpointer user_data) {
	char* argv[] = {
		"cmd-polkit-agent", 
		"-sp",
		"-c", 
		" ", 
		NULL
	};
	app__init(4, argv);
	g_assert_cmpstr(get_stderr()->str, ==, "\
Error parsing command line: error parsing shell command: Text was empty (or contained only whitespace)\n\
\n\
"HELP_MESSAGE"\
");
	g_assert_cmpstr(get_stdout()->str, ==, "");
}


int main (int argc, char *argv[]) {

    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

	#define test(path, func)   g_test_add (path, Fixture, NULL, test_set_up, func, test_tear_down);

    // Define the tests.
    test ("/ request messages / all request messages are single line", test_all_request_messages_are_single_line);
    test ("/ request messages / request message request password is escaped correctly", test_request_message_request_password_is_escaped_correctly);
    test ("/ logger / default level logs failure and normal logs", test_default_logs);
	test ("/ logger / silenced level logs nothing", test_silenced_logs);
	test ("/ logger / verbose level logs all logs", test_verbose_logs);
	test ("/ logger / polkit authentication logs", test_log_verbose_init_polkit_authentication);
	test ("/ logger / polkit authentication identity information is logged as json", test_log_polkit_auth_identities);
	test ("/ logger / invalid polkit authentication identity is still logged as json, to help identify the cause of a failure", test_log_invalid_polkit_auth_identities);
	test ("/ logger / polkit action description logs", test_log_polkit_action_description);
	test ("/ logger / empty policy kit details is shown as empty", test_log_empty_polkit_details);
	test ("/ logger / non-empty policy kit details is shown with key: value pairs", test_log_non_empty_polkit_details);
	test ("/ logger / log_polkit_session_completed logs session information", test_log_polkit_session_completed);
	test ("/ accepted actions / accepted_action_value_of_str returns expected value on valid action", test_accepted_action_value_of_str_returns_expected_values_on_valid_actions);
	test ("/ accepted actions / accepted_action_value_of_str returns UNKNOWN on invalid action", test_accepted_action_value_of_str_returns_unknown_on_invalid_actions);
	test ("/ accepted actions / accepted_action_value_of_str is case sensitive", test_accepted_action_value_of_str_is_case_sensitive);
	test ("/ json glib extensions / json_node_get_string_or_else returns string value or else value if not a string", test_json_node_get_string_or_else);
	test ("/ json glib extensions / json_object_get_string_member_or_else returns string value or else value if not a string", test_json_node_get_string_member_or_else);
	test ("/ error message dialog / lazy gtk init inits gtk once", test_error_message_dialog_inits_once);
	test ("/ error message dialog / show message runs gtk dialog", test_error_message_dialog_runs_gtk_dialog);
	test ("/ polkit auth handler / CmdPkAgentPolkitListenerClass initialization", test_polkit_auth_handler_CmdPkAgentPolkitListenerClass_init);
	test ("/ app / app initialization without arguments shows help message", test_app_init_without_arguments_shows_help_message);
	test ("/ app / app initialization with invalid arguments shows help message", test_app_init_with_invalid_arguments_shows_help_message);
	test ("/ app / app initialization without serial and parallel mmodes shows error", test_app_init_without_serial_and_parallel_shows_error);
	test ("/ app / app initialization with both serial and parallel modes shows error", test_app_init_with_both_serial_and_parallel_shows_error);
	test ("/ app / app initialization with blank command line shows error", test_app_init_with_blank_command_line_shows_error);

	#undef test
	return g_test_run ();
}