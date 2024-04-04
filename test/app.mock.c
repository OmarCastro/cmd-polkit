// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#include "app.mock.h"
#include "../src/app.c"
#include <stdbool.h>

void app__reset(){
	isInitialized = false;
	if(cmd_line != NULL){
		g_free(cmd_line);
		cmd_line = NULL;
	}
	if(cmd_line_argv != NULL){
	    g_strfreev (cmd_line_argv);
		cmd_line_argv = NULL;
	}
}
