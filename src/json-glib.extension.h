// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2020 Omar Castro
#ifndef EXTENSION__H__JSON_GLIB
#define EXTENSION__H__JSON_GLIB

#include <json-glib/json-glib.h>

gboolean json_node_get_boolean_or_else(JsonNode * node, gboolean else_value);

const gchar * json_node_get_string_or_else(JsonNode * node, const gchar * else_value);

gboolean json_object_get_boolean_member_or_else(JsonObject * node, const gchar * member, gboolean else_value);

const gchar * json_object_get_string_member_or_else(JsonObject * node, const gchar * member, const gchar * else_value);

#endif //EXTENSION__H__JSON_GLIB