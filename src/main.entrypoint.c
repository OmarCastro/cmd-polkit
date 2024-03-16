// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#include <stdlib.h>
#include <stdbool.h>
#include "polkit-auth-handler.service.h"
#include "cmdline.h"
#include "logger.h"
#include "app.h"
#include "error-message.dialog.h"

static bool runInSerie;
static bool runInParallel;
static bool runInSilence;
static bool runInVerbose;
static char *cmd_line = NULL;
static int static_argc;
static char ** static_argv;


const char*  app__get_cmd_line(){
  return cmd_line;
}

AuthHandlingMode app__get_auth_handling_mode(){
  return runInParallel ? AuthHandlingMode_PARALLEL : AuthHandlingMode_SERIE;
}


Application app_get(){
  return (Application){
      .argc = static_argc ,
      .argv = static_argv ,
      .command_line = cmd_line ,
      .handling_mode = runInParallel ? AuthHandlingMode_PARALLEL : AuthHandlingMode_SERIE
  };
}

int main(int argc, char *argv[])
{


  static_argc = argc;
  static_argv = argv;

  struct gengetopt_args_info ai;
  if (cmdline_parser(argc, argv, &ai) != 0) {
    cmdline_parser_print_help();
    exit(1);
  }

  PolkitAgentListener *listener;
  PolkitSubject* session;
  GError* error = NULL;
  GMainLoop *loop;


  runInSerie = ai.serial_given;
  runInParallel = ai.parallel_given;
  runInSilence = ai.quiet_given || ai.silent_given;
  runInVerbose = !runInSilence && ai.verbose_given;

  if(runInSilence){
    log__silence();
  }

  if(runInVerbose){
    log__verbose();
  }



  if( !ai.command_given ){
    log__fail_cmdline__command_required();
    exit(1);
  }

  cmd_line = ai.command_arg;

  char **cmd_argv = NULL;

  if ( !g_shell_parse_argv ( ai.command_arg, NULL, &cmd_argv, &error ) ){
      fprintf(stderr, "Unable to parse cmdline options: %s\n", error->message);
      g_error_free ( error );
      return 0;
  }

  g_strfreev(cmd_argv);

  if(runInSerie && runInParallel){
    log__fail_cmdline__either_parallel_or_series();
    exit(1);
  }

  if(!runInSerie && !runInParallel){
    log__fail_cmdline__parallel_or_series_required();
    exit(1);
  }

  log__verbose__cmd_and_mode();





    loop = g_main_loop_new (NULL, FALSE);

    int rc = 0;

    listener = cmd_pk_agent_polkit_listener_new();
    session = polkit_unix_session_new_for_process_sync(getpid(), NULL, NULL);

    if(!polkit_agent_listener_register(listener,
                       POLKIT_AGENT_REGISTER_FLAGS_NONE,
                       session, NULL, NULL, &error)) {

        show_error_message_format("Error initializing program\n %s\nThe application will exit", error->message);
        fprintf(stderr,"Error %s",error->message);
        g_error_free ( error );
        rc = 1;
    } else {
      g_main_loop_run (loop);
    }

    g_object_unref(listener);
    g_object_unref(session);
    g_main_loop_unref (loop);
    return rc;
}
