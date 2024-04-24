// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#define LOGGER_C
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <grp.h>
#include <pwd.h>
#include <polkit/polkit.h>
#include "cmdline.h"
#include "app.h"
#include "logger.h"
#include <json-glib/json-glib.h>


bool silenced_logs = false;
bool verbose_logs = false;


const char * currentFile = "";
const char * currentFunction = "";
int currentLine = 0;

#define UPDATE_CURRENT_SOURCE_LOCATION() currentFile = file; currentFunction = function; currentLine = line;
#define CHECK_VERBOSE() if(!verbose_logs || silenced_logs) { return; }


struct LogMessage {};

static void log__fail_cmdline(const char* text);
static void log__verbose_formatted(const char* format, ...);
static inline void log__verbose_raw(const char* text){ log__verbose_formatted("%s", text); }


void print_help (FILE *file)
{
	size_t len_purpose = strlen(gengetopt_args_info_purpose);
	size_t len_usage = strlen(gengetopt_args_info_usage);

	if (len_usage > 0) {
		fprintf(file, "%s\n", gengetopt_args_info_usage);
	}
	if (len_purpose > 0) {
		fprintf(file, "%s\n", gengetopt_args_info_purpose);
	}

	if (len_usage || len_purpose) {
		fprintf(file, "\n");
	}

	if (strlen(gengetopt_args_info_description) > 0) {
		fprintf(file, "%s\n\n", gengetopt_args_info_description);
	}

  int i = 0;
  while (gengetopt_args_info_help[i])
    fprintf(file, "%s\n", gengetopt_args_info_help[i++]);
}


void log__silence(){
  silenced_logs = true;
}
void log__verbose(){
  verbose_logs = true;
}

// log error, print help and exits

void log__fail_cmdline__command_required(MACRO__SOURCE_LOCATION_PARAMS){
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__fail_cmdline("command argument is required");
}

void log__fail_cmdline__either_parallel_or_series(MACRO__SOURCE_LOCATION_PARAMS){
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__fail_cmdline("only serial or parallel mode must be selected, not both");
}

void log__fail_cmdline__parallel_or_series_required(MACRO__SOURCE_LOCATION_PARAMS){
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__fail_cmdline("parallel or serial mode is required");
}

void log__fail_cmdline__print_help(){
  fprintf(stderr, "\n");
  print_help(stderr);
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

void log__verbose__finalize_polkit_listener(MACRO__SOURCE_LOCATION_PARAMS){
  UPDATE_CURRENT_SOURCE_LOCATION()
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


const gchar * polkit_auth_identity_to_json_string(PolkitIdentity * identity);
void log__verbose__polkit_auth_identities(MACRO__SOURCE_LOCATION_PARAMS, const GList* const identities){
  CHECK_VERBOSE()
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__verbose_raw("Polkit identities");
  for (const GList *p = identities; p != NULL; p = p->next) {
      PolkitIdentity *id = (PolkitIdentity *)p->data;
      g_autofree const gchar* json = polkit_auth_identity_to_json_string(id);
      log__verbose_formatted("└─ %s", json);
    }
}

void log__verbose__polkit_auth_details(MACRO__SOURCE_LOCATION_PARAMS, PolkitDetails* const details){
  CHECK_VERBOSE()
  UPDATE_CURRENT_SOURCE_LOCATION()
  char** keys = polkit_details_get_keys(details);

  log__verbose_raw("Polkit details");
  if (keys == NULL) { 
    log__verbose_raw("└─ (empty)");
    return; 
  }
  for(char** key = keys;*key;key++) {
      log__verbose_formatted("└─ %s: %s", *key, polkit_details_lookup(details, *key));
  }
  g_strfreev(keys);
}


void log__verbose__polkit_session_completed(MACRO__SOURCE_LOCATION_PARAMS, bool authorized, bool canceled){
  CHECK_VERBOSE()
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__verbose_raw("Polkit session completed");
  log__verbose_formatted("└─ {\"authorized\": \"%s\", \"canceled\":\"%s\" })", authorized ? "yes": "no", canceled ? "yes": "no");
}

void log__verbose__polkit_session_show_error(MACRO__SOURCE_LOCATION_PARAMS, const char *text){
  CHECK_VERBOSE()
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__verbose_formatted("Polkit session show error: %s", text);
}


void log__verbose__polkit_session_show_info(MACRO__SOURCE_LOCATION_PARAMS, const char *text){
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__verbose_formatted("Polkit session show info: %s", text);
}


void log__verbose__polkit_session_request(MACRO__SOURCE_LOCATION_PARAMS, const char *text, bool visibility){
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__verbose_formatted("Polkit session request: %s", text);
  log__verbose_formatted("└─ visibility: %s", visibility ? "yes" : "no");

}

void log__verbose__finish_polkit_authentication(MACRO__SOURCE_LOCATION_PARAMS){
  UPDATE_CURRENT_SOURCE_LOCATION()
  log__verbose_raw("finish Polkit authentication");
}

void log__verbose__writing_to_command_stdin(MACRO__SOURCE_LOCATION_PARAMS, const char * message){
    UPDATE_CURRENT_SOURCE_LOCATION()
    log__verbose_formatted("writing to command stdin: %s", message);
}

void log__verbose__received_from_command_stdout(MACRO__SOURCE_LOCATION_PARAMS, const char * message){
    UPDATE_CURRENT_SOURCE_LOCATION()
    log__verbose_formatted("received line from command stdout: %s", message);
}

void log__verbose__reading_command_stdout(MACRO__SOURCE_LOCATION_PARAMS){
    UPDATE_CURRENT_SOURCE_LOCATION()
    log__verbose_formatted("reading output");
}



// private

static void log__verbose_formatted( const char* format, ... )
{
  CHECK_VERBOSE()
  va_list arglist;
  printf( "Vrbos:%s:", currentFunction );
  va_start( arglist, format );
  vprintf( format, arglist );
  va_end( arglist );
  printf( "\n" );

}

static void log__fail_cmdline(const char* text){
  if(!silenced_logs) {
    fprintf(stderr, "Error parsing command line: %s\n", text);
    log__fail_cmdline__print_help();
  }
}


const gchar * polkit_auth_identity_to_json_string(PolkitIdentity * identity){
    g_autoptr(JsonBuilder) builder = json_builder_new ();

    json_builder_begin_object (builder);

    if(identity == NULL){
      json_builder_set_member_name (builder, "type");
      json_builder_add_string_value (builder, "error");

      json_builder_set_member_name (builder, "error");
      json_builder_add_string_value (builder, "identity is null");
    } else if(POLKIT_IS_UNIX_USER(identity)) {
      uid_t uid = polkit_unix_user_get_uid(POLKIT_UNIX_USER(identity));
      struct passwd *pwd = getpwuid(uid);
      json_builder_set_member_name (builder, "type");
      json_builder_add_string_value (builder, "user");

      json_builder_set_member_name (builder, "name");
      json_builder_add_string_value (builder, pwd->pw_name);

      json_builder_set_member_name (builder, "id");
      json_builder_add_int_value (builder, pwd->pw_uid);

      json_builder_set_member_name (builder, "group id");
      json_builder_add_int_value (builder, pwd->pw_gid);

    } else if(POLKIT_IS_UNIX_GROUP(identity)) {
      gid_t gid = polkit_unix_group_get_gid(POLKIT_UNIX_GROUP(identity));
      struct group *grp = getgrgid(gid);

      json_builder_set_member_name (builder, "type");
      json_builder_add_string_value (builder, "group");

      json_builder_set_member_name (builder, "name");
      json_builder_add_string_value (builder, grp->gr_name);

      json_builder_set_member_name (builder, "id");
      json_builder_add_int_value (builder, grp->gr_gid);
    } else if(POLKIT_IS_IDENTITY(identity)){
      g_autofree gchar* value = polkit_identity_to_string(identity);
      
      json_builder_set_member_name (builder, "type");
      json_builder_add_string_value (builder, "other");

      json_builder_set_member_name (builder, "value");
      json_builder_add_string_value (builder, value);
    } else {
      json_builder_set_member_name (builder, "type");
      json_builder_add_string_value (builder, "error");

      json_builder_set_member_name (builder, "error");
      json_builder_add_string_value (builder, "invalid type: not a polkit identity");
    }

    json_builder_end_object (builder);
    g_autoptr(JsonNode) root = json_builder_get_root (builder);
    g_autoptr(JsonGenerator) gen = json_generator_new ();
    json_generator_set_root (gen, root);
    return json_generator_to_data (gen, NULL);
}
