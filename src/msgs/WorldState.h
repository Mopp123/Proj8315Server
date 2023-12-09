#pragma once

#include "../../Proj8315Common/src/Common.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "Client.h"

class Server;

namespace msgs
{
    gamecommon::Message get_all_factions(Server& server, const Client& client,  gamecommon::Message& msg);
    gamecommon::Message create_new_faction(Server& server, const Client& client, gamecommon::Message& msg);
    gamecommon::Message edit_faction(Server& server, const Client& client, gamecommon::Message& msg);
}
