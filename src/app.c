// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#include <stdbool.h>
#include <stdlib.h>
#include "app.h"
#include "cmdline.h"
#include "logger.h"

static AuthHandlingMode handling_mode;
static char *cmd_line = NULL;
static int static_argc;
static char ** static_argv;
static bool isInitialized = false;

const char*  app__get_cmd_line(){
  return cmd_line;
}

typedef struct 
{
	int argc;
	char ** argv;
	const char* command_line;
	AuthHandlingMode handling_mode;
} Application;


AuthHandlingMode app__get_auth_handling_mode(){
  return handling_mode;
}

int app__get_argc(){
  return static_argc;
}
char ** app__get_argv(){
  return static_argv;
}

int app__init(int argc, char *argv[]){
	if(isInitialized){
		return 0;
	}

  static_argc = argc;
  static_argv = argv;

  struct gengetopt_args_info ai;
  if (cmdline_parser(argc, argv, &ai) != 0) {
    cmdline_parser_print_help();
    exit(1);
  }

  bool runInSerie = ai.serial_given;
  bool runInParallel = ai.parallel_given;
  bool runInSilence = ai.quiet_given || ai.silent_given;
  bool runInVerbose = !runInSilence && ai.verbose_given;

  if(runInSilence){
    log__silence();
  }

  if(runInVerbose){
    log__verbose();
  }



  if( !ai.command_given ){
    log__fail_cmdline__command_required();
    return 1;
  }

  cmd_line = ai.command_arg;

  char **cmd_argv = NULL;
  GError* error = NULL;

  if ( !g_shell_parse_argv ( ai.command_arg, NULL, &cmd_argv, &error ) ){
      fprintf(stderr, "Unable to parse cmdline options: %s\n", error->message);
      g_error_free ( error );
      return 1;
  }

  g_strfreev(cmd_argv);

  if(runInSerie && runInParallel){
    log__fail_cmdline__either_parallel_or_series();
    return 1;
  }

  if(!runInSerie && !runInParallel){
    log__fail_cmdline__parallel_or_series_required();
    return 1;
  }

  handling_mode = runInParallel ? AuthHandlingMode_PARALLEL : AuthHandlingMode_SERIE;

  log__verbose__cmd_and_mode();
  return 0;
}