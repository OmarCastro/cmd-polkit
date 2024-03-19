// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#include <string.h>
#include "accepted-actions.enum.h"

const char * AcceptedAction_CANCEL_str_value = "cancel";
const char * AcceptedAction_AUTHENTICATE_str_value = "authenticate";

AcceptedAction accepted_action_value_of_str(const char * str){
	if(str == NULL){
		return AcceptedAction_UNKNOWN;
	}
    switch (str[0]) {
        case 'c':
            return strcmp(str, AcceptedAction_CANCEL_str_value) == 0 ? AcceptedAction_CANCEL: AcceptedAction_UNKNOWN;
        case 'a':
            return strcmp(str, AcceptedAction_AUTHENTICATE_str_value) == 0 ? AcceptedAction_AUTHENTICATE: AcceptedAction_UNKNOWN;
    }
    return AcceptedAction_UNKNOWN;
}
