// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#ifndef APP_H
#define APP_H

typedef enum {
   AuthHandlingMode_SERIE,
   AuthHandlingMode_PARALLEL
} AuthHandlingMode;

typedef struct 
{
	int argc;
	char ** argv;
	const char* command_line;
	AuthHandlingMode handling_mode;
} Application;

Application app_get();

const char*  app__get_cmd_line();
AuthHandlingMode app__get_auth_handling_mode();

#endif

