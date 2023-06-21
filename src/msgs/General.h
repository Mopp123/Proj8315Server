#pragma once


#include "../../Proj8315Common/src/Common.h"
#include "../../Proj8315Common/src/Message.h"
#include "Client.h"


class Server;


namespace msgs
{
    gamecommon::Message get_server_message(Server& server, const Client& client, gamecommon::Message& msg);
    gamecommon::Message user_login(Server& server, const Client& client, gamecommon::Message& msg);
    gamecommon::Message user_register(Server& server, const Client& client, gamecommon::Message& msg);
    gamecommon::Message fetch_obj_type_lib(Server& server, const Client& client, gamecommon::Message& msg);

    gamecommon::Message update_observer(Server& server, const Client& client, gamecommon::Message& msg);

    //Message server_shutdown(Server& server, Message& msg);
}
