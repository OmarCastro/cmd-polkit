// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#include "json-glib.extension.h"

const gchar * json_node_get_string_or_else(JsonNode * node, const gchar * else_value){
    return node != NULL &&
           json_node_get_value_type(node) == G_TYPE_STRING ?
           json_node_get_string(node) : else_value;
}

const gchar * json_object_get_string_member_or_else(JsonObject * node, const gchar * member, const gchar * else_value){
    return json_node_get_string_or_else(json_object_get_member(node, member), else_value);
}