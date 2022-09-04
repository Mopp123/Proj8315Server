#pragma once

#include <string>

#include "Common.h"
#include "MessageHandler.h"

class Server;

namespace msgs
{
	Message msg_createNewFaction(Server& server, Message& msg);
	Message msg_getServerMessage(Server& server, Message& msg);
	Message msg_updateObserver(Server& server, Message& msg);
	Message msg_serverShutdown(Server& server, Message& msg);
}
