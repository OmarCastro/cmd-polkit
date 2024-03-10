#include "tap.h"
#include "../src/app.h"
#include <stdbool.h>
#include <glib.h>
#include <locale.h>


const char* cmd_line = "test_test.sh";
static bool runInParallel;
 

AuthHandlingMode app__get_auth_handling_mode(){
  return runInParallel ? AuthHandlingMode_PARALLEL : AuthHandlingMode_SERIE;
}


const char*  app__get_cmd_line(){
  return cmd_line;
}

typedef struct {
  int obj;
} MyObjectFixture;


static void
my_object_fixture_set_up (MyObjectFixture *fixture,
                          gconstpointer user_data)
{

}

static void
my_object_fixture_tear_down (MyObjectFixture *fixture,
                             gconstpointer user_data)
{
}

static void
test_my_object_test1 (MyObjectFixture *fixture,
                      gconstpointer user_data)
{
      g_assert_cmpstr ("initil", ==, "initial-value");
  }



int main (int argc, char *argv[]) {
    runInParallel = true;


    setlocale (LC_ALL, "");

  g_test_init (&argc, &argv, NULL);

  // Define the tests.
  g_test_add ("/my-object/test1", MyObjectFixture, "some-user-data",
              my_object_fixture_set_up, test_my_object_test1,
              my_object_fixture_tear_down);

  return g_test_run ();
}