// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#ifndef ENUM__H__ACCEPTED_ACTIONS
#define ENUM__H__ACCEPTED_ACTIONS

typedef enum {
   AcceptedAction_CANCEL = 1,
   AcceptedAction_AUTHENTICATE = 2,
   
   AcceptedAction_UNKNOWN = 0
} AcceptedAction;

AcceptedAction accepted_action_value_of_str(const char * str);

#endif //EXTENSION__H__JSON_GLIB