#include "polkit.mock.h"


static void request(gpointer session){
          g_signal_emit_by_name (session, "request", "Password:", FALSE);
}


static void complete_success(gpointer session){
  g_signal_emit_by_name (session, "completed", TRUE);
}

void mock_polkit_agent_session_initiate (PolkitAgentSession *session){
    g_idle_add_once(request, session);

}

void mock_polkit_agent_session_cancel (PolkitAgentSession *session)
{
    g_signal_emit_by_name (session, "completed", FALSE);
}

void mock_polkit_agent_session_response (PolkitAgentSession *session, const gchar *response)
{
        g_idle_add_once(complete_success, session);
}


