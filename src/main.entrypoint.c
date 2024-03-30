// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#include <stdlib.h>
#include <stdbool.h>
#include "polkit-auth-handler.service.h"
#include <glib.h>
#include <stdio.h>
#include "app.h"
#include "error-message.dialog.h"


int main(int argc, char *argv[])
{

  const int return_code = app__init(argc, argv);
  if(return_code != 0){
    return return_code;
  }
  PolkitAgentListener *listener;
  PolkitSubject* session;
  GError* error = NULL;
  GMainLoop *loop;

    loop = g_main_loop_new (NULL, FALSE);

    int rc = 0;

    listener = cmd_pk_agent_polkit_listener_new();
    session = polkit_unix_session_new_for_process_sync(getpid(), NULL, NULL);

    if(!polkit_agent_listener_register(listener,
                       POLKIT_AGENT_REGISTER_FLAGS_NONE,
                       session, NULL, NULL, &error)) {

        show_error_message_format("Error initializing program\n %s\nThe application will exit", error->message);
        fprintf(stderr,"Error %s",error->message);
        g_error_free ( error );
        rc = 1;
    } else {
      g_main_loop_run (loop);
    }

    g_object_unref(listener);
    g_object_unref(session);
    g_main_loop_unref (loop);
    return rc;
}
