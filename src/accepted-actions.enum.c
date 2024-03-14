// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#include <gmodule.h>
#include "accepted-actions.enum.h"


static GHashTable * table;

const char * AcceptedAction_CANCEL_str_value = "cancel";
const char * AcceptedAction_AUTHENTICATE_str_value = "authenticate";

AcceptedAction accepted_action_value_of_str(const char * str){
	if(str == NULL){
		return AcceptedAction_UNKNOWN;
	}

    if(table == NULL){
        table = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, NULL);
        g_hash_table_insert(table, (gpointer) AcceptedAction_CANCEL_str_value, GINT_TO_POINTER(AcceptedAction_CANCEL));
        g_hash_table_insert(table, (gpointer) AcceptedAction_AUTHENTICATE_str_value, GINT_TO_POINTER(AcceptedAction_AUTHENTICATE));
    }

    gpointer pointer = g_hash_table_lookup(table, str);
    if(pointer == NULL){
        return AcceptedAction_UNKNOWN;
    } else {
        return (AcceptedAction)(GPOINTER_TO_INT(pointer)) ;
    }
}
