#pragma once

#include <cstring>


#define FACTION_DATA_MAX_STRLEN 32

class Faction
{
private:

	 char _name[FACTION_DATA_MAX_STRLEN];

public:

	Faction(const char* name, size_t nameLen)
	{
		if (nameLen > FACTION_DATA_MAX_STRLEN)
			nameLen = FACTION_DATA_MAX_STRLEN;
		memset(_name, 0, sizeof(char) * FACTION_DATA_MAX_STRLEN);
		memcpy(_name, name, sizeof(char) * nameLen);
	}
	
	Faction(const Faction& other) = delete;

	const char* getName() const { return _name; }

	bool operator==(const Faction& other) { return strcmp(_name, other._name) == 0; }
};
