#pragma once

#include <cstdint>
#include <unordered_map>
#include "game/objects/ObjectManager.h"


namespace world
{
    // Param: waterPoolSizes is mapping containing contiguous body of water's tile count for each water tile
    // ..so u may know how big body of water from a single water tile...
    void generate_environment(
        uint64_t* pWorld,
        int worldWidth,
        const std::unordered_map<int, int>& waterPoolSizes,
        objects::ObjectManager* pObjManager
    );
}
