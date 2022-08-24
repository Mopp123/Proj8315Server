#pragma once

#include <string>
#include "MessageHandler.h"

namespace msgs
{
	Message msg_createNewFaction(const Message& msg);
	Message msg_fetchServerMessage(const Message& msg);
	Message msg_serverShutdown(const Message& msg);
}
