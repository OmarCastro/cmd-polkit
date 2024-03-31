// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#include "app.mock.h"
#include "../src/app.c"
#include <stdbool.h>

void app__reset(){
	isInitialized = false;
}
