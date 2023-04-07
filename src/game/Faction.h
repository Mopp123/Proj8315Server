#pragma once

#include "Common.h"
#include <cstring>
#include <string>


#define FACTION_NAME_SIZE 32
#define FACTION_MAX_DEPLOY_COUNT 16

#define NULL_FACTION Faction("", 0)

class Faction
{
private:
    PK_byte _nameData[FACTION_NAME_SIZE];

    // Contains what objects faction spawns, when spawning to the map
    PK_ubyte _deployments[FACTION_MAX_DEPLOY_COUNT];

    // If faction's data is changed -> broadcast its new data to all clients
    bool _updated = true;
    std::string _name;

public:
    Faction(const PK_byte* nameData, size_t nameSize);
    Faction(const Faction& other) = delete;

    void setDeployments(PK_ubyte* deployments, size_t count);
    const PK_byte* getNetwData() const;
    static size_t get_netw_size();

    inline const PK_byte* getNameData() const { return _nameData; }
    inline const std::string& getName() const { return _name; }
    inline const PK_ubyte* getDeploymens() { return _deployments; }
    inline bool isUpdated() const { return _updated; }
    inline void markUpdated(bool arg) { _updated = arg; }

    bool operator==(const Faction& other) { return memcmp(_nameData, other._nameData, FACTION_NAME_SIZE) == 0; }
};
