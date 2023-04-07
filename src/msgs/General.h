#pragma once

#include <string>

#include "Common.h"
#include "MessageHandler.h"


class Server;


namespace msgs
{
    Message get_server_message(Server& server, Message& msg);
    Message user_login(Server& server, Message& msg);
    Message user_register(Server& server, Message& msg);
    Message fetch_obj_type_lib(Server& server, Message& msg);

    Message update_observer(Server& server, Message& msg);

    //Message server_shutdown(Server& server, Message& msg);
}
