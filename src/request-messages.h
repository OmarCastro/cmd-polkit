// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#ifndef EXTENSION__H__REQUEST_MESSAGES
#define EXTENSION__H__REQUEST_MESSAGES

#include "polkit/polkittypes.h"
#include <polkit/polkit.h>
/**
 * Sent when password given is correct
 */
const gchar * request_message_authorization_authorized();

const gchar * request_message_authorization_not_authorized();

const gchar * request_message_request_password(
    const gchar * prompt,
    const gchar * message,
    PolkitActionDescription* action_description
);

const gchar * request_message_show_info(const gchar * text);

const gchar * request_message_show_error(const gchar * text);

#endif //EXTENSION__H__REQUEST_MESSAGES