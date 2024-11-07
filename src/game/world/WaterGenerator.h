#pragma once

#include <set>
#include <cstdint>
#include <unordered_map>


namespace world
{
    // Used when generating "water areas/pools of water" using DFS in below func
    struct PoolSearchNode
    {
        int x = 0;
        int y = 0;
        int parentIndex = 0;
    };

    // Returns indices to tiles that are continuously at same level with tile at x, y
    // (leaks into lower tiles if found)
    std::set<int> generate_water_area(
        uint64_t* pWorld,
        int worldWidth,
        int x,
        int y,
        int& waterLevel
    );

    // Param: Pool count means how many "pools of water" we create
    // Returns: mapping of "water pool sizes".
    // This contains each water tile's(key) pool's tile count.
    // This means how big contiguous body of water for each water tile
    std::unordered_map<int, int> generate_world_waters(
        uint64_t* pWorld,
        int worldWidth,
        int poolCount
    );
}
