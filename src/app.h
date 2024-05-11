// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#ifndef APP_H
#define APP_H

typedef enum {
   AuthHandlingMode_SERIE,
   AuthHandlingMode_PARALLEL
} AuthHandlingMode;


int app__init(int argc, char *argv[]);
int app__get_argc();
char ** app__get_argv();
const char*  app__get_cmd_line();
char** app__get_cmd_line_argv();
AuthHandlingMode app__get_auth_handling_mode();

#endif

