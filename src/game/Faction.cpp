#include "Faction.h"


Faction::Faction(const char* name, size_t nameLen)
{
	if (nameLen > FACTION_DATA_MAX_STRLEN)
		nameLen = FACTION_DATA_MAX_STRLEN;
	memset(_name, 0, sizeof(char) * FACTION_DATA_MAX_STRLEN);
	memcpy(_name, name, sizeof(char) * nameLen);

	memset(_deployments, 0, sizeof(PK_ubyte) * FACTION_DATA_MAX_DEPLOY_COUNT);
}

void Faction::setDeployments(PK_ubyte* deployments, size_t count) 
{
	if (count <= FACTION_DATA_MAX_DEPLOY_COUNT)
		memcpy(_deployments, deployments, sizeof(PK_ubyte) * count);
}
