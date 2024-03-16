#include <stdbool.h>
#include <glib.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include "../src/app.h"
#include "src/logger.h"

GString *memory_stdout = NULL;
AuthHandlingMode mode = AuthHandlingMode_PARALLEL;

static void mock_printf(const char * format, ...){

  va_list arglist;
  va_start( arglist, format );
  g_string_vprintf( memory_stdout, format, arglist );
  va_end( arglist );

}

// mock
#define printf(f_, ...) mock_printf((f_), ##__VA_ARGS__)


// mock
const char*  app__get_cmd_line(){
  return "command test";
}

// mock
void cmdline_parser_print_help(void) {
	g_string_printf(memory_stdout, "<mock help message>\n");
}

AuthHandlingMode app__get_auth_handling_mode(){
  return mode;
}


#include "../src/logger.c"
#include "../src/request-messages.c"


typedef struct {
  int obj;
} Fixture;


static void test_set_up (Fixture *fixture, gconstpointer user_data){
	if(memory_stdout == NULL){
		memory_stdout = g_string_new("");
	} else {
		g_string_erase(memory_stdout, 0, -1);
	}
	silenced_logs = false
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

static void test_silenced_logs (Fixture *fixture, gconstpointer user_data) {
	log__silence()
	log__fail_cmdline()
}




int main (int argc, char *argv[]) {

    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

    // Define the tests.
    g_test_add ("/ request messages / all request messages are single line", Fixture, NULL, test_set_up, test_all_request_messages_are_single_line, test_tear_down);
    g_test_add ("/ request messages / request message request password is escaped correctly", Fixture, NULL, test_set_up, test_request_message_request_password_is_escaped_correctly, test_tear_down);
    g_test_add ("/ logger / silenced logs logs nothing", Fixture, NULL, test_set_up, test_request_message_request_password_is_escaped_correctly, test_tear_down);

  return g_test_run ();
}