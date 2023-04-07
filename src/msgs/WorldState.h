#pragma once

#include "MessageHandler.h"

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

    Message get_all_factions(Server& server, Message& msg);
    Message create_new_faction(Server& server, Message& msg);
    Message edit_faction(Server& server, Message& msg);
}
