#pragma once

#include <string>
#include "Responses.h"
#include "Commands.h"


namespace cmds
{
	Response cmd_createNewFaction(const Command& cmd);
	Response cmd_fetchServerMessage(const Command& cmd);
	
}
