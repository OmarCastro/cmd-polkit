#define POLKIT_AGENT_I_KNOW_API_IS_SUBJECT_TO_CHANGE
#include <polkitagent/polkitagent.h>
#include <polkit/polkit.h>


void mock_polkit_agent_session_initiate (PolkitAgentSession *session);
void mock_polkit_agent_session_cancel (PolkitAgentSession *session);
void mock_polkit_agent_session_response (PolkitAgentSession *session, const gchar *response);


#define polkit_agent_session_initiate(session) mock_polkit_agent_session_initiate(session)
#define polkit_agent_session_cancel(session) mock_polkit_agent_session_cancel(session)
#define polkit_agent_session_response(session,response) mock_polkit_agent_session_response(session,response)
