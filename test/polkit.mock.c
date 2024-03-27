#include "polkit.mock.h"

static int request(gpointer session){
  g_signal_emit_by_name (session, "request", "Password:", FALSE);
	return G_SOURCE_REMOVE;
}

static int complete_success(gpointer session){
  g_signal_emit_by_name (session, "completed", TRUE);
  return G_SOURCE_REMOVE;
}

static int complete_error(gpointer session){
  g_signal_emit_by_name (session, "completed", FALSE);
  return G_SOURCE_REMOVE;
}

void mock_polkit_agent_session_initiate (PolkitAgentSession *session){
  g_idle_add(request, session);
}

void mock_polkit_agent_session_cancel (PolkitAgentSession *session){
  g_idle_add(complete_error, session);
}

void mock_polkit_agent_session_response (PolkitAgentSession *session, const gchar *response){
  const gboolean authenticated = g_str_equal(response, "success");
  if(authenticated){
    g_idle_add(complete_success, session);
  } else {
    g_timeout_add(500, complete_error, session);
  }
}


