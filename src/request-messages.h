// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#ifndef EXTENSION__H__REQUEST_MESSAGES
#define EXTENSION__H__REQUEST_MESSAGES

#include <glib.h>
/**
 * Sent when password given is correct
 */
const gchar * request_message_authorization_authorized();

const gchar * request_message_authorization_not_authorized();

const gchar * request_message_request_password(const gchar * prompt, const gchar * message);


#endif //EXTENSION__H__REQUEST_MESSAGES