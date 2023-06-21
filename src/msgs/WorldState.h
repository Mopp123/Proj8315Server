#pragma once

#include "../../Proj8315Common/src/Common.h"
#include "../../Proj8315Common/src/Message.h"
#include "Client.h"

class Server;

namespace msgs
{
    /*
        Format:
            0 = type(int32_t)
            1 = x pos (int32_t)
            2 = y pos (int32_t)
            3 = radius (int32_t)
    */
    // TODO: Remove -> this wasnt used after moving from requests to websockets
    // Message msg_fetchWorldState(Server& server, Message& msg);

    gamecommon::Message get_all_factions(Server& server, const Client& client,  gamecommon::Message& msg);
    gamecommon::Message create_new_faction(Server& server, const Client& client, gamecommon::Message& msg);
    gamecommon::Message edit_faction(Server& server, const Client& client, gamecommon::Message& msg);
}
