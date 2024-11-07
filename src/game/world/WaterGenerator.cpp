#include "WaterGenerator.h"
#include "../../../Proj8315Common/src/Common.h"
#include "../../../Proj8315Common/src/Tile.h"
#include <vector>


using namespace gamecommon;


namespace world
{
    // Returns indices to tiles that are continuously at same level with tile at x, y
    // (leaks into lower tiles if found)
    std::set<int> generate_water_area(uint64_t* pWorld, int worldWidth, int x, int y, int& waterLevel)
    {
        // Iterate through adjacent tiles that are at same elevation by depth first search
        // TODO: optimize..

        // NOTE:
        // *This vector is used as "stack" so we use the last element kind of like our first
        std::vector<PoolSearchNode> searchStack;
        // Indices of visited tiles
        std::set<int> visited;
        searchStack.push_back({ x, y, 0 });
        while (true)
        {
            if(searchStack.empty())
                break;

            PoolSearchNode observe = searchStack.back();
            searchStack.pop_back();
            const int observeIndex = observe.x + observe.y * worldWidth;

            if (visited.find(observeIndex) != visited.end())
                continue;

            PoolSearchNode adjacentNodes[4] =
            {
                { observe.x - 1, observe.y, 0 },
                { observe.x + 1, observe.y, 0 },
                { observe.x, observe.y - 1, 0 },
                { observe.x, observe.y + 1, 0 }
            };

            GC_ubyte currentHeight = get_tile_terrelevation(*(pWorld + observeIndex));
            for (int i = 0; i < 4; ++i)
            {
                const int adjacentIndex = adjacentNodes[i].x + adjacentNodes[i].y * worldWidth;
                if (visited.find(adjacentIndex) != visited.end())
                    continue;
                if (adjacentNodes[i].x >= 0 && adjacentNodes[i].x < worldWidth && adjacentNodes[i].y >= 0 && adjacentNodes[i].y < worldWidth)
                {
                    GC_ubyte adjacentHeight = get_tile_terrelevation(*(pWorld + adjacentIndex));
                    if (adjacentHeight == currentHeight)
                    {
                        searchStack.push_back(adjacentNodes[i]);
                    }
                    // If tile was lower -> leak into it
                    else if (adjacentHeight < currentHeight)
                    {
                        return generate_water_area(pWorld, worldWidth, adjacentNodes[i].x, adjacentNodes[i].y, waterLevel);
                    }
                    else if (adjacentHeight > currentHeight)
                    {
                        if (waterLevel != -1)
                            waterLevel = std::min(waterLevel, (int)adjacentHeight);
                        else
                            waterLevel = adjacentHeight;
                    }
                }
            }

            visited.insert(observeIndex);
        }
        return visited;
    }

    // *Pool count means how many "pools of water" we create
    std::unordered_map<int, int> generate_world_waters(uint64_t* pWorld, int worldWidth, int poolCount)
    {
        std::srand(666);
        // key = water tiles, value = the "water pool's" tile count
        // -> can be used to see how big body of contiquous water
        std::unordered_map<int, int> tilePoolSizeMapping;

        // pair's first = tile index, second = height the water should be at
        //std::set<std::pair<int, GC_ubyte>> finalWaters;

        for (int i = 0; i < poolCount; ++i)
        {
            int rx = std::rand() % worldWidth;
            int ry = std::rand() % worldWidth;
            int waterLevel = -1;
            std::set<int> waterTiles = generate_water_area(
                pWorld,
                worldWidth,
                rx,
                ry,
                waterLevel
            );
            if (waterLevel == -1) // This shouldn't happen tho..?
                waterLevel = 0;

            for (const int tileIndex: waterTiles)
            {
                GC_ubyte h = waterLevel;
                if (h > 0)
                    h -= 1;

                uint64_t& tileRef = *(pWorld + tileIndex);
                set_tile_terrtype(tileRef, TileStateTerrType::TILE_STATE_terrTypeWater);
                set_tile_terrelevation(tileRef, h);

                // ...quite dumb I know... need that for now...
                tilePoolSizeMapping[tileIndex] = waterTiles.size();
            }
            // NOTE: No idea why the fuck didn't just set the water tiles immediately before?
            //for (int tileIndex : waterTiles)
            //    finalWaters.insert(std::make_pair(tileIndex, (GC_ubyte)waterLevel));
        }

        // NOTE: No idea why the fuck didn't just set the water tiles immediately before?
        /*
        for (const std::pair<int, GC_ubyte>& p : finalWaters)
        {
            GC_ubyte h = p.second;
            if (h > 0)
                h -= 1;
            set_tile_terrtype(*(pWorld + p.first), TileStateTerrType::TILE_STATE_terrTypeWater);
            set_tile_terrelevation(*(pWorld + p.first), h);
        }
        */
        return tilePoolSizeMapping;
    }
}
