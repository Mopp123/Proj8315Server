#pragma once

#include "Common.h"
#include <cstring>


#define FACTION_DATA_MAX_STRLEN 32
#define FACTION_DATA_MAX_DEPLOY_COUNT 16


class Faction
{
private:

	 char _name[FACTION_DATA_MAX_STRLEN];

	 // Contains what objects faction spawns, when spawning to the map
	 PK_ubyte _deployments[FACTION_DATA_MAX_DEPLOY_COUNT];

public:

	Faction(const char* name, size_t nameLen);
	Faction(const Faction& other) = delete;

	void setDeployments(PK_ubyte* deployments, size_t count);

	inline const char* getName() const { return _name; }
	inline const PK_ubyte* getDeploymens() { return _deployments; }

	bool operator==(const Faction& other) { return strcmp(_name, other._name) == 0; }
};
