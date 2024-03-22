// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#ifndef LOGGER_H
#define LOGGER_H
#include <stdbool.h>
#include <glib.h>
#include <polkit/polkittypes.h>

#define MACRO__SOURCE_LOCATION_PARAMS const char* file, const char* function, const int line
#define MACRO__SOURCE_LOCATION_VALUES __FILE__, __func__ ,__LINE__

void log__silence();
void log__verbose();

// log error, print help and exits

void log__fail_cmdline__command_required(MACRO__SOURCE_LOCATION_PARAMS);
void log__fail_cmdline__either_parallel_or_series(MACRO__SOURCE_LOCATION_PARAMS);
void log__fail_cmdline__parallel_or_series_required(MACRO__SOURCE_LOCATION_PARAMS);

#ifndef LOGGER_C
#define log__fail_cmdline__command_required()               log__fail_cmdline__command_required(MACRO__SOURCE_LOCATION_VALUES)
#define log__fail_cmdline__either_parallel_or_series()      log__fail_cmdline__either_parallel_or_series(MACRO__SOURCE_LOCATION_VALUES)
#define log__fail_cmdline__parallel_or_series_required()    log__fail_cmdline__parallel_or_series_required(MACRO__SOURCE_LOCATION_VALUES)
#endif

// logs on verbose only

void log__verbose__cmd_and_mode(MACRO__SOURCE_LOCATION_PARAMS);
void log__verbose__init_polkit_listener(MACRO__SOURCE_LOCATION_PARAMS);
void log__verbose__init_polkit_authentication(MACRO__SOURCE_LOCATION_PARAMS, const char *action_id, const char *message, const char *icon_name, const char * cookie );
void log__verbose__polkit_auth_identities(MACRO__SOURCE_LOCATION_PARAMS, const GList* const list);
void log__verbose__polkit_auth_details(MACRO__SOURCE_LOCATION_PARAMS, PolkitDetails* const details);
void log__verbose__polkit_session_completed(MACRO__SOURCE_LOCATION_PARAMS, bool authorized, bool canceled);
void log__verbose__polkit_session_show_error(MACRO__SOURCE_LOCATION_PARAMS, const char *text);
void log__verbose__polkit_session_show_info(const char *text);
void log__verbose__polkit_session_request(const char *text, bool visibility);
void log__verbose__finish_polkit_authentication();
void log__verbose__finalize_polkit_listener();
void log__verbose__writing_to_command_stdin(const char * message);
void log__verbose__received_from_command_stdout(const char * message);
void log__verbose__reading_command_stdout();




#ifndef LOGGER_C
#define log__verbose__cmd_and_mode()                   log__verbose__cmd_and_mode(MACRO__SOURCE_LOCATION_VALUES)
#define log__verbose__init_polkit_listener()           log__verbose__init_polkit_listener(MACRO__SOURCE_LOCATION_VALUES)
#define log__verbose__init_polkit_authentication(action_id, message, icon_name, cookie) \
        log__verbose__init_polkit_authentication(MACRO__SOURCE_LOCATION_VALUES, action_id, message, icon_name, cookie)
#define log__verbose__polkit_auth_identities(list)     log__verbose__polkit_auth_identities(MACRO__SOURCE_LOCATION_VALUES, list)
#define log__verbose__polkit_auth_details(details)     log__verbose__polkit_auth_details(MACRO__SOURCE_LOCATION_VALUES, details)
#define log__verbose__polkit_session_completed(authorized, canceled) \
        log__verbose__polkit_session_completed(MACRO__SOURCE_LOCATION_VALUES, authorized, canceled)
#define log__verbose__polkit_session_show_error(text)  log__verbose__polkit_session_show_error(MACRO__SOURCE_LOCATION_VALUES, text)

#endif

#endif // LOGGER_H
