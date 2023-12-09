#include "WorldState.h"
#include "Server.h"
#include "game/Game.h"


using namespace gamecommon;

namespace msgs
{
    Message get_all_factions(Server& server, const Client& client, Message& msg)
    {
        return Game::get()->getAllFactions();
    }

    Message create_new_faction(Server& server, const Client& client, Message& msg)
    {
        const size_t msgSize = msg.getDataSize();
        if(msgSize >= MESSAGE_MIN_DATA_SIZE + FACTION_NAME_SIZE)
        {
            const GC_byte* data = msg.getData();

            GC_byte factionNameData[FACTION_NAME_SIZE];
            memset(factionNameData, 0, FACTION_NAME_SIZE);
            memcpy(factionNameData, data + MESSAGE_ENTRY_SIZE__header, FACTION_NAME_SIZE);

            return Game::get()->addFaction(
                server,
                client,
                factionNameData,
                FACTION_NAME_SIZE
            );
        }
        else
        {
            return NULL_MESSAGE;
        }
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
