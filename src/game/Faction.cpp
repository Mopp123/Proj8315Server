#include "Faction.h"
#include "Debug.h"


Faction::Faction(const PK_byte* nameData, size_t nameSize)
{
    if (nameSize > FACTION_NAME_SIZE)
        nameSize = FACTION_NAME_SIZE;
    memset(_nameData, 0, FACTION_NAME_SIZE);
    memcpy(_nameData, nameData, nameSize);

    memset(_deployments, 0, sizeof(PK_ubyte) * FACTION_MAX_DEPLOY_COUNT);

    _name = std::string(_nameData, FACTION_NAME_SIZE);
}

void Faction::setDeployments(PK_ubyte* deployments, size_t count)
{
    if (count <= FACTION_MAX_DEPLOY_COUNT)
        memcpy(_deployments, deployments, sizeof(PK_ubyte) * count);
}

const PK_byte* Faction::getNetwData() const
{
    return _nameData;
}

size_t Faction::get_netw_size()
{
    return FACTION_NAME_SIZE;
}

