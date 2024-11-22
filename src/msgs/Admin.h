#pragma once

#include "../../Proj8315Common/src/Common.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "Client.h"


class Server;


namespace msgs
{
    gamecommon::Message spawn_object(Server& server, const Client& client, gamecommon::Message& msg);
    gamecommon::Message terrain_modification(Server& server, const Client& client, gamecommon::Message& msg);
}
