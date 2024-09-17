#include "WorldState.h"
#include "Server.h"
#include "game/Game.h"
#include "../../Proj8315Common/src/messages/WorldMessages.h"


using namespace gamecommon;

namespace msgs
{
    Message get_all_factions(Server& server, const Client& client, Message& msg)
    {
        Message response = Game::get()->getAllFactions();
        return response;
    }

    Message create_new_faction(Server& server, const Client& client, Message& msg)
    {
        CreateFactionRequest createFactionReq(msg.getData(), msg.getDataSize());
        if (createFactionReq != NULL_MESSAGE)
        {
            return Game::get()->addFaction(
                server,
                client,
                createFactionReq.getName()
            );
        }
        return NULL_MESSAGE;
    }

    Message edit_faction(Server& server, const Client& client, Message& msg)
    {
        /*
            1. Get message's client's user's faction
            2. Update it with the incomming data
        */
        const size_t msgSize = msg.getDataSize();
        if(msgSize >= MESSAGE_MIN_DATA_SIZE + Faction::get_netw_size())
        {
        }
        return NULL_MESSAGE;
    }
}
