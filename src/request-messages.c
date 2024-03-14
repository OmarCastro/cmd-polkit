// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#include <json-glib/json-glib.h>
#include "request-messages.h"


const gchar * request_message_authorization_authorized(){
    return g_strdup("{\"action\":\"authorization response\",\"authorized\":true}");
}

const gchar * request_message_authorization_not_authorized(){
    return g_strdup("{\"action\":\"authorization response\",\"authorized\":false}");

}

const gchar * request_message_request_password(const gchar * prompt, const gchar * message){
    g_autoptr(JsonBuilder) builder = json_builder_new ();

    json_builder_begin_object (builder);

    json_builder_set_member_name (builder, "action");
    json_builder_add_string_value (builder, "request password");

    json_builder_set_member_name (builder, "prompt");
    json_builder_add_string_value (builder, prompt);

    json_builder_set_member_name (builder, "message");
    json_builder_add_string_value (builder, message);

    json_builder_end_object (builder);

    g_autoptr(JsonNode) root = json_builder_get_root (builder);

    g_autoptr(JsonGenerator) gen = json_generator_new ();
    json_generator_set_root (gen, root);
    return json_generator_to_data (gen, NULL);
}

