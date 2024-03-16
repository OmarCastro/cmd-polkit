// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#ifndef LOGGER__TEST_H
#define LOGGER__TEST_H
#include <glib.h>

GString * get_stdout();
GString * get_stderr();
void reset_logs();


#endif // LOGGER__TEST_H
