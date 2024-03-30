// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#ifndef APP_MOCK_H
#define APP_MOCK_H

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


int app_init(int argc, char *argv[]);
Application app_get();

const char*  app__get_cmd_line();
AuthHandlingMode app__get_auth_handling_mode();

#endif

