#include <stdbool.h>
#include <glib.h>
#include <locale.h>
#include <string.h>
#include "../src/request-messages.h"


typedef struct {
  int obj;
} Fixture;


static void test_set_up (Fixture *fixture, gconstpointer user_data){}

static void test_tear_down (Fixture *fixture, gconstpointer user_data){}

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




int main (int argc, char *argv[]) {

    setlocale (LC_ALL, "");

    g_test_init (&argc, &argv, NULL);

    // Define the tests.
    g_test_add ("/ request messages / all request messages are single line", Fixture, NULL, test_set_up, test_all_request_messages_are_single_line, test_tear_down);
    g_test_add ("/ request messages / request message request password is escaped correctly", Fixture, NULL, test_set_up, test_request_message_request_password_is_escaped_correctly, test_tear_down);

  return g_test_run ();
}