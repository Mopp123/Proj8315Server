#pragma once

#include <cstring>


#define FACTION_DATA_MAX_STRLEN 32

class Faction
{
private:

	 char _name[FACTION_DATA_MAX_STRLEN];

public:

	Faction(const char* name)
	{
		memcpy(_name, name, FACTION_DATA_MAX_STRLEN);
	}
	
	Faction(const Faction& other) = delete;

	const char* getName() const { return _name; }

	bool operator==(const Faction& other) { return strcmp(_name, other._name) == 0; }
};
