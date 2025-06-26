// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#include <json-glib/json-glib.h>
#include <time.h>
#include "request-messages.h"


const gchar * request_message_authorization_authorized(){
    return g_strdup("{\"action\":\"authorization response\",\"authorized\":true}");
}

const gchar * request_message_authorization_not_authorized(){
    return g_strdup("{\"action\":\"authorization response\",\"authorized\":false}");

}

const gchar * request_message_request_password(    
    const gchar * prompt,
    const gchar * message,
    PolkitActionDescription* action_description
){
    g_autoptr(JsonBuilder) builder = json_builder_new ();

    json_builder_begin_object (builder);

    json_builder_set_member_name (builder, "action");
    json_builder_add_string_value (builder, "request password");

    json_builder_set_member_name (builder, "prompt");
    json_builder_add_string_value (builder, prompt);

    json_builder_set_member_name (builder, "message");
    json_builder_add_string_value (builder, message);

    json_builder_set_member_name(builder, "polkit action");
    if(action_description == NULL){
        json_builder_add_null_value(builder);
    } else {
        json_builder_begin_object (builder);
        json_builder_set_member_name (builder, "id");
        json_builder_add_string_value (builder, polkit_action_description_get_action_id(action_description));

        json_builder_set_member_name (builder, "description");
        json_builder_add_string_value (builder, polkit_action_description_get_description(action_description));

        json_builder_set_member_name (builder, "message");
        json_builder_add_string_value (builder, polkit_action_description_get_message(action_description));

        json_builder_set_member_name (builder, "vendor name");
        json_builder_add_string_value (builder, polkit_action_description_get_vendor_name(action_description));

        json_builder_set_member_name (builder, "vendor url");
        json_builder_add_string_value (builder, polkit_action_description_get_vendor_url(action_description));

        json_builder_set_member_name (builder, "icon name");
        json_builder_add_string_value (builder, polkit_action_description_get_icon_name(action_description));


        json_builder_end_object (builder);

    }

    json_builder_end_object (builder);

    g_autoptr(JsonNode) root = json_builder_get_root (builder);

    g_autoptr(JsonGenerator) gen = json_generator_new ();
    json_generator_set_root (gen, root);
    return json_generator_to_data (gen, NULL);
}

const gchar * request_message_show_info(const gchar * text){
    g_autoptr(JsonBuilder) builder = json_builder_new ();

    json_builder_begin_object (builder);

    json_builder_set_member_name (builder, "action");
    json_builder_add_string_value (builder, "show info");

    json_builder_set_member_name (builder, "text");
    json_builder_add_string_value (builder, text);
   
    json_builder_end_object (builder);

    g_autoptr(JsonNode) root = json_builder_get_root (builder);

    g_autoptr(JsonGenerator) gen = json_generator_new ();
    json_generator_set_root (gen, root);
    return json_generator_to_data (gen, NULL);
}

const gchar * request_message_show_error(const gchar * text){
    g_autoptr(JsonBuilder) builder = json_builder_new ();

    json_builder_begin_object (builder);

    json_builder_set_member_name (builder, "action");
    json_builder_add_string_value (builder, "show error");

    json_builder_set_member_name (builder, "text");
    json_builder_add_string_value (builder, text);
   
    json_builder_end_object (builder);

    g_autoptr(JsonNode) root = json_builder_get_root (builder);

    g_autoptr(JsonGenerator) gen = json_generator_new ();
    json_generator_set_root (gen, root);
    return json_generator_to_data (gen, NULL);
}