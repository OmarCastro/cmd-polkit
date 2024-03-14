// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2024 Omar Castro
#ifndef SERVICE__H__POLKIT_AUTHENTICATION_HANDLER
#define SERVICE__H__POLKIT_AUTHENTICATION_HANDLER

#define POLKIT_AGENT_I_KNOW_API_IS_SUBJECT_TO_CHANGE
#include <polkitagent/polkitagent.h>
#include <polkit/polkit.h>

G_BEGIN_DECLS

#define CMD_PK_AGENT_TYPE_POLKIT_LISTENER             (cmd_pk_agent_polkit_listener_get_type())
#define CMD_PK_AGENT_POLKIT_LISTENER(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), CMD_PK_AGENT_TYPE_POLKIT_LISTENER, CmdPkAgentPolkitListener))
#define CMD_PK_AGENT_POLKIT_LISTENER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), CMD_PK_AGENT_TYPE_POLKIT_LISTENER, CmdPkAgentPolkitListenerClass))
#define CMD_PK_AGENT_IS_POLKIT_LISTENER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), CMD_PK_AGENT_TYPE_POLKIT_LISTENER))
#define CMD_PK_AGENT_IS_POLKIT_LISTENER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), CMD_PK_AGENT_TYPE_POLKIT_LISTENER))
#define CMD_PK_AGENT_POLKIT_LISTENER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), CMD_PK_AGENT_TYPE_POLKIT_LISTENER, CmdPkAgentPolkitListenerClass))


typedef struct _CmdPkAgentPolkitListener CmdPkAgentPolkitListener;
typedef struct _CmdPkAgentPolkitListenerClass CmdPkAgentPolkitListenerClass;

struct _CmdPkAgentPolkitListener {
		PolkitAgentListener parent;
};

struct _CmdPkAgentPolkitListenerClass {
		PolkitAgentListenerClass parent_class; 
};

GType cmd_pk_agent_polkit_listener_get_type(void);
PolkitAgentListener* cmd_pk_agent_polkit_listener_new(void);

G_END_DECLS

#endif

