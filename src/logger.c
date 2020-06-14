#define LOGGER_C
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <grp.h>
#include <pwd.h>
#include "polkit-auth-handler.service.h"
#include "cmdline.h"
#include "app.h"
#include "logger.h"


bool silenced = false;
bool verbose = false;


const char * currentFile = "";
const char * currentFunction = "";
int currentLine = 0;

#define UPDATE_CURRENT_SOURCE_LOCATION() currentFile = file; currentFunction = function; currentLine = line;


struct LogMessage {};

static void log_stderr_text(const char* text);
static void log__fail_cmdline(const char* text);
static void log__verbose_formatted(const char* format, ...);
static inline void log__verbose_raw(const char* text){ log__verbose_formatted("%s", text); }



void log__silence(){
  silenced = true;
}
void log__verbose(){
  verbose = true;
}

// log error, print help and exits

void log__fail_cmdline__command_required(MACRO__SOURCE_LOCATION_PARAMS){
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__fail_cmdline("command argument is required");
}

void log__fail_cmdline__either_parallel_or_series(MACRO__SOURCE_LOCATION_PARAMS){
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__fail_cmdline("Only parallel or serial must be selected, not both");
}

void log__fail_cmdline__parallel_or_series_required(MACRO__SOURCE_LOCATION_PARAMS){
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__fail_cmdline("Parallel or serial option are required");
}


// logs on verbose only

void log__verbose__cmd_and_mode(MACRO__SOURCE_LOCATION_PARAMS){
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__verbose_formatted("COMMAND TO EXECUTE: %s", app__get_cmd_line());
  log__verbose_formatted("AUTH HANDLING MODE: %s", app__get_auth_handling_mode() == AuthHandlingMode_PARALLEL ? "PARALLEL" : "SERIE");
}

void log__verbose__init_polkit_listener(MACRO__SOURCE_LOCATION_PARAMS){
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__verbose_raw("Polkit Listener initialized");
}

void log__verbose__finalize_polkit_listener(){
  log__verbose_raw("Polkit Listener finalized");
}

void log__verbose__init_polkit_authentication(MACRO__SOURCE_LOCATION_PARAMS, const char *action_id, const char *message, const char *icon_name, const char * cookie ){
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__verbose_raw("initiate Polkit authentication");
  log__verbose_formatted("└─ action id : %s", action_id);
  log__verbose_formatted("└─ message   : %s", message);
  log__verbose_formatted("└─ icon name : %s", icon_name);
  log__verbose_formatted("└─ cookie    : %s", cookie);

}


void log__verbose__polkit_auth_identities(MACRO__SOURCE_LOCATION_PARAMS, const GList* const identities){
  UPDATE_CURRENT_SOURCE_LOCATION()
  const GList *p;

  log__verbose_raw("polkit identities");
  for (p = identities; p != NULL; p = p->next) {
      PolkitIdentity *id = (PolkitIdentity *)p->data;
      if(POLKIT_IS_UNIX_USER(id)) {
          uid_t uid = polkit_unix_user_get_uid(POLKIT_UNIX_USER(id));
          struct passwd *pwd = getpwuid(uid);
          log__verbose_formatted(R"""(└- {"type": "user", "name":"%s", "id": %d, "group id": %d })""", pwd->pw_name, pwd->pw_uid, pwd->pw_gid);
        } else if(POLKIT_IS_UNIX_GROUP(id)) {
          gid_t gid = polkit_unix_group_get_gid(POLKIT_UNIX_GROUP(id));
          struct group *grp = getgrgid(gid);
          log__verbose_formatted(R"""(└- {"type": "group", "name": "%s", "uid":%d })""", grp->gr_name, grp->gr_gid);
        } else {
          log__verbose_formatted(R"""(└- {"type": "other", "value":"%s" })""", polkit_identity_to_string(id));
        }

    }
}

void log__verbose__polkit_session_completed(bool authorized, bool canceled){
  log__verbose_raw("polkit session completed");
  log__verbose_formatted(R"""(└- {"authorized": "%s", "canceled":"%s" })""", authorized ? "yes": "no", canceled ? "yes": "no");
}

void log__verbose__polkit_session_show_error(const char *text){
  log__verbose_formatted("Polkit session show error: %s", text);
}


void log__verbose__polkit_session_show_info(const char *text){
  log__verbose_formatted("Polkit session show info: %s", text);
}


void log__verbose__polkit_session_request(const char *text, bool visibility){
  log__verbose_formatted("Polkit session request: %s", text);
  log__verbose_formatted("└- visibility: %s", visibility ? "yes" : "no");

}

void log__verbose__finish_polkit_authentication(){
  log__verbose_raw("finish Polkit authentication");
}

void log__verbose__writing_to_command_stdin(const char * message){
    log__verbose_formatted("writing to command stdin: %s", message);
}

void log__verbose__received_from_command_stdout(const char * message){
    log__verbose_formatted("received line from command stdout: %s", message);
}

void log__verbose__reading_command_stdout(){
    log__verbose_formatted("reading output");
}



// private

static void log__verbose_formatted( const char* format, ... )
{

  if(!verbose || silenced){
    return;
  }
  va_list arglist;
  printf( "Vrbos:%s:", currentFunction );
  va_start( arglist, format );
  vprintf( format, arglist );
  va_end( arglist );
  printf( "\n" );

}


static void log_stderr_text(const char* text){
  if(!silenced){
    fprintf(stderr, "Error: %s\n", text);
  }
}

static void log__fail_cmdline(const char* text){
  log_stderr_text(text);
  cmdline_parser_print_help();
  exit(1);
}

