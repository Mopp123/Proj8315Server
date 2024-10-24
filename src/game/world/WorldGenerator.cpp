#include "WorldGenerator.h"
#include "../../../Proj8315Common/src/Common.h"
#include "../../../Proj8315Common/src/Tile.h"
#include "../../utils/Algorithm.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <set>
#include "../../Debug.h"


using namespace gamecommon;

namespace world
{
    static bool is_tile_flat(uint64_t* pWorld, int worldWidth, int x, int y, GC_ubyte currentElevation)
    {
        for(int y2 = y - 1; y2 <= y + 1; ++y2)
        {
            for(int x2 = x - 1; x2 <= x + 1; ++x2)
            {
                const int adjIndex = x2 + y2 * worldWidth;
                if(adjIndex >= 0 && adjIndex < worldWidth * worldWidth)
                {
                    if(get_tile_terrelevation(pWorld[x2 + y2 * worldWidth]) != currentElevation)
                        return false;
                }
            }
        }
        return true;
    }


    static void create_ramp(
        uint64_t* pWorld,
        int worldWidth,
        int posX,
        int posY,
        int rampWidth
    )
    {
        GC_ubyte rampBeginHeight = get_tile_terrelevation(*(pWorld + (posX + posY * worldWidth)));
        int jCount = rampBeginHeight;
        int iCount = rampWidth;

        TileStateDirection direction = TileStateDirection::TILE_STATE_dirN;

        // Figure out which direction ramp should descent
        int rampBuildThreshold = 2;
        int dirWeights[8];
        memset(dirWeights, 0, sizeof(int) * 8);
        // Also keep track of the adjacent tile heights to figure out what our target height should be
        // after direction has been figured
        GC_ubyte dirHeights[8];
        memset(dirHeights, 0, 8);
        int dirHeightsIndex = 0;
        for (int y = posY - 1; y <= posY + 1; ++y)
        {
            for (int x = posX - 1; x <= posX + 1; ++x)
            {
                if (x < 0 || x >= worldWidth || y < 0 || y >= worldWidth || (x == posX && y == posY))
                    continue;

                GC_ubyte adjacentElevation = get_tile_terrelevation(*(pWorld + (x + y * worldWidth)));

                dirHeights[dirHeightsIndex] = adjacentElevation;
                ++dirHeightsIndex;

                if ((int)rampBeginHeight -  (int)adjacentElevation >= rampBuildThreshold)
                {
                    // north weight
                    if (y == posY - 1)
                        dirWeights[TileStateDirection::TILE_STATE_dirN] += 1 +dirWeights[TileStateDirection::TILE_STATE_dirN];
                    // south weight
                    else if (y == posY + 1)
                        dirWeights[TileStateDirection::TILE_STATE_dirS] += 1 + dirWeights[TileStateDirection::TILE_STATE_dirS];

                    // east weight
                    if (x == posX + 1)
                        dirWeights[TileStateDirection::TILE_STATE_dirE] += 1 + dirWeights[TileStateDirection::TILE_STATE_dirE];
                    // west weight
                    if (x == posX - 1)
                        dirWeights[TileStateDirection::TILE_STATE_dirW] += 1 + dirWeights[TileStateDirection::TILE_STATE_dirW];
                }
            }
        }
        // sort dirHeights to mach direction indexing
        GC_ubyte swapTarget[8] = { 1, 2, 4, 7, 6, 5, 3, 0 };
        GC_ubyte sortedDirHeights[8];
        for (int i = 0; i < 8; ++i)
            sortedDirHeights[i] = dirHeights[swapTarget[i]];

        // calc diagonal dir weights
        dirWeights[TileStateDirection::TILE_STATE_dirNE] = (dirWeights[TileStateDirection::TILE_STATE_dirN] + dirWeights[TileStateDirection::TILE_STATE_dirE]) / 2;
        dirWeights[TileStateDirection::TILE_STATE_dirSE] = (dirWeights[TileStateDirection::TILE_STATE_dirS] + dirWeights[TileStateDirection::TILE_STATE_dirE]) / 2;
        dirWeights[TileStateDirection::TILE_STATE_dirSW] = (dirWeights[TileStateDirection::TILE_STATE_dirS] + dirWeights[TileStateDirection::TILE_STATE_dirW]) / 2;
        dirWeights[TileStateDirection::TILE_STATE_dirNW] = (dirWeights[TileStateDirection::TILE_STATE_dirN] + dirWeights[TileStateDirection::TILE_STATE_dirW]) / 2;
        // Prioritize diagonal by adding 1 to non zeros
        for (int i = 1; i <= 7; i += 2)
            dirWeights[i] = dirWeights[i] > 0 ? dirWeights[i] + 1 : dirWeights[i];

        int highestVal = 0;
        int highestDir = 0;
        for (int i = 0; i < 8; ++i)
        {
            int val = dirWeights[i];
            if (val > highestVal)
            {
                highestVal = val;
                highestDir = i;
            }
        }
        direction = (TileStateDirection)highestDir;

        // Figure out what the target height should be
        GC_ubyte targetHeight = sortedDirHeights[highestDir];

        bool diagonal = direction == TileStateDirection::TILE_STATE_dirNE ||
            direction == TileStateDirection::TILE_STATE_dirSE ||
            direction == TileStateDirection::TILE_STATE_dirSW ||
            direction == TileStateDirection::TILE_STATE_dirNW;

        // Need to make a bit wider if diagonal..
        if (diagonal)
        {
            iCount += 1;
            rampWidth += 1;
        }

        // Center the ramp little better in following cases.. looks better..
        if (direction == TileStateDirection::TILE_STATE_dirN || direction == TileStateDirection::TILE_STATE_dirS)
            posX -= rampWidth / 2;
        if (direction == TileStateDirection::TILE_STATE_dirE || direction == TileStateDirection::TILE_STATE_dirW)
            posY -= rampWidth / 2;

        for (int j = 0; j < jCount; ++j)
        {
            for (int i = 0; i < iCount; ++i)
            {
                int tileY = posY + j;
                int tileX = posX + i;
                int steepnessModifier = j * 2;

                if (direction == TileStateDirection::TILE_STATE_dirN)
                {
                    tileY = posY - j;
                }
                else if (direction == TileStateDirection::TILE_STATE_dirW)
                {
                    tileY = posY + i;
                    tileX = posX - j;
                }
                else if (direction == TileStateDirection::TILE_STATE_dirE)
                {
                    tileY = posY + i;
                    tileX = posX + j;
                }
                // Diagonal cases are a bit.. special..
                else if (direction == TileStateDirection::TILE_STATE_dirNE)
                {
                    tileY = posY - j;
                    tileX = posX + (i + j) - (std::ceil(rampWidth / 2));
                    steepnessModifier = j * 2 + i;
                }
                else if (direction == TileStateDirection::TILE_STATE_dirSE)
                {
                    tileY = posY + j;
                    tileX = posX + (i + j) - (std::ceil(rampWidth / 2));
                    steepnessModifier = j * 2 + i;
                }
                else if (direction == TileStateDirection::TILE_STATE_dirSW)
                {
                    tileY = posY + j;
                    tileX = posX - ((rampWidth - 1) - i) - j + (std::ceil(rampWidth / 2));
                    steepnessModifier = j * 2 + ((rampWidth - 1) - i);
                }
                else if (direction == TileStateDirection::TILE_STATE_dirNW)
                {
                    tileY = posY - j;
                    tileX = posX - (i + j) + (std::ceil(rampWidth / 2));
                    steepnessModifier = j * 2 + i;
                }

                if (tileX < 0 || tileX >= worldWidth || tileY < 0 || tileY >= worldWidth)
                {
                    continue;
                }

                int finalHeight = (int)rampBeginHeight - steepnessModifier;

                if (finalHeight >= targetHeight)
                {
                    const GC_ubyte h = (GC_ubyte)finalHeight;
                    uint64_t& targetTile = *(pWorld + (tileX + tileY * worldWidth));
                    set_tile_terrelevation(targetTile, h);
                    set_tile_terrtype(targetTile, 4);
                }
            }
        }
    }


    static void generate_world_elevation(uint64_t* pWorld, int worldWidth, int maxElevationVal, unsigned int seed)
    {
        srand(seed);

        size_t seedArrLength = worldWidth * worldWidth;
        float* seedArr = new float[seedArrLength];
        memset(seedArr, 0, sizeof(float) * seedArrLength);
        for(size_t i = 0; i < seedArrLength; ++i)
            seedArr[i] = (float)(std::rand() % maxElevationVal) / maxElevationVal;

        int octaveCount = 6;
        float scaleDivisor = 0.4f;
        //int octaveCount = 5;
        //float scaleDivisor = 0.0125f;

        float minHeight = 0.0f;
        std::vector<float> heightmap = generate_perlin2D(seedArr, worldWidth, octaveCount, scaleDivisor, &minHeight);
        Debug::log("min height was: " + std::to_string(minHeight));
        delete[] seedArr;

        // Copy the elevation from heightmap to our pWorld
        for(int y = 0; y < worldWidth; ++y)
        {
            for(int x = 0; x < worldWidth; ++x)
            {
                size_t index = x + y * worldWidth;
                float fHeight = heightmap[index];

                GC_ubyte h = (GC_ubyte)(fHeight * (float)maxElevationVal);

                set_tile_terrelevation(pWorld[index], h);
            }
        }
    }


    static std::vector<float> create_heightmap(
        int width,
        unsigned int seed,
        int octaveCount,
        float scaleDivisor,
        int maxElevationVal
    )
    {
        srand(seed);

        size_t seedArrLength = width * width;
        float* seedArr = new float[seedArrLength];
        memset(seedArr, 0, sizeof(float) * seedArrLength);
        for(size_t i = 0; i < seedArrLength; ++i)
            seedArr[i] = (float)(std::rand() % maxElevationVal) / maxElevationVal;

        float minHeight = 0.0f;
        std::vector<float> heightmap = generate_perlin2D(seedArr, width, octaveCount, scaleDivisor, &minHeight);
        delete[] seedArr;

        return heightmap;
    }


    static void generate_world_elevation_TEST(uint64_t* pWorld, int worldWidth, int maxElevationVal, unsigned int seed)
    {
        // first one is more "broad/general" shape of the world
        std::vector<float> heightmap1 = create_heightmap(worldWidth, seed, 5, 0.4f, maxElevationVal);
        // second is more "detailed"
        std::vector<float> heightmap2 = create_heightmap(worldWidth, seed + 1, 7, 0.05f, maxElevationVal);

        // each different "elevation levels" go by increments of this value
        const int elevationMultiplier = 4;
        int elevationLevels = std::floor((float)maxElevationVal / (float)elevationMultiplier);
        //int maxPlateau = elevationLevels * elevationMultiplier;

        // Copy the elevation from heightmap to our pWorld
        for(int y = 0; y < worldWidth; ++y)
        {
            for(int x = 0; x < worldWidth; ++x)
            {
                size_t index = x + y * worldWidth;

                float fHeight = lerp(heightmap1[index], heightmap2[index], 0.25f);
                //float fHeight = heightmap[index];

                GC_ubyte h = std::round(fHeight / (1.0f / (float)elevationLevels)) * elevationMultiplier;

                //GC_ubyte h = (GC_ubyte)(fHeight * (float)maxElevationVal);

                set_tile_terrelevation(pWorld[index], h);
                set_tile_terrtype(pWorld[index], 4);
            }
        }
    }

    // Should we also generate ramps here?
    static void generate_world_erosion(uint64_t* pWorld, int worldWidth, int cliffThreshold)
    {
        for(int y = 0; y < worldWidth; ++y)
        {
            for(int x = 0; x < worldWidth; ++x)
            {
                const int check_radius = 1;
                int currentIndex = x + y * worldWidth;
                int currentElevation = (int)get_tile_terrelevation(pWorld[currentIndex]);
                for(int y2 = y - check_radius; y2 <= y + check_radius; ++y2)
                {
                    for(int x2 = x - check_radius; x2 <= x + check_radius; ++x2)
                    {
                        const int adjIndex = x2 + y2 * worldWidth;
                        if(adjIndex >= 0 && adjIndex < worldWidth * worldWidth)
                        {
                            int adjElevation = (int)get_tile_terrelevation(pWorld[adjIndex]);
                            if (adjElevation > currentElevation)
                            {
                                set_tile_terrtype(pWorld[currentIndex], 3);
                                set_tile_terrtype(pWorld[adjIndex], 3);
                            }
                        }
                    }
                }
            }
        }
    }


    static void generate_world_waters(uint64_t* pWorld, int worldWidth)
    {
        const GC_ubyte elev_threshold_water = 5;
        const GC_ubyte elev_cap_fertile = 16;
        const int fertilityFrequency = 8;
        // "calc erosion / water pools"
        std::unordered_set<int> tilesToSkip;
        for(int y = 0; y < worldWidth; ++y)
        {
            for(int x = 0; x < worldWidth; ++x)
            {
                int index = x + y * worldWidth;
                GC_ubyte tileElevation = get_tile_terrelevation(pWorld[index]);
                if(tileElevation > elev_threshold_water && tileElevation < elev_cap_fertile)
                {
                    int diceThrow = std::rand() % (tileElevation * tileElevation * tileElevation) / fertilityFrequency;
                    if(diceThrow <= elev_cap_fertile)
                        set_tile_terrtype(pWorld[index], 4);
                    continue;
                }

                if(tilesToSkip.find(index) != tilesToSkip.end())
                    continue;
                if(tileElevation <= elev_threshold_water)
                {
                    set_tile_terrelevation(pWorld[index], elev_threshold_water);
                    for(int y2 = y - 1; y2 <= y + 1; ++y2)
                    {
                        for(int x2 = x - 1; x2 <= x + 1; ++x2)
                        {
                            int adjacentIndex = x2 + y2 * worldWidth;
                            if(adjacentIndex >= 0 && adjacentIndex < worldWidth * worldWidth)
                            {
                                GC_ubyte t = get_tile_terrelevation(pWorld[adjacentIndex]);
                                if(t != tileElevation)
                                {
                                    set_tile_terrelevation(pWorld[adjacentIndex], elev_threshold_water);
                                    tilesToSkip.insert(adjacentIndex);
                                }
                            }
                        }
                    }
                }

            }
        }
        // then place the actual water tiles in the "pools"
        for(int y = 0; y < worldWidth; ++y)
        {
            for(int x = 0; x < worldWidth; ++x)
            {
                int index = x + y * worldWidth;
                GC_ubyte tileElevation = get_tile_terrelevation(pWorld[index]);
                if(tileElevation <= elev_threshold_water)
                {
                    bool isFlat = true;
                    for(int y2 = y - 1; y2 <= y + 1; ++y2)
                    {
                        for(int x2 = x - 1; x2 <= x + 1; ++x2)
                        {
                            int adjacentIndex = x2 + y2 * worldWidth;
                            if(adjacentIndex >= 0 && adjacentIndex < worldWidth * worldWidth)
                            {
                                GC_ubyte t = get_tile_terrelevation(pWorld[adjacentIndex]);
                                if(t != tileElevation)
                                {
                                    isFlat = false;
                                    break;
                                }
                            }
                        }
                        if(!isFlat)
                            break;
                    }
                    if(isFlat)
                        set_tile_terrtype(pWorld[index], 1);
                }

            }
        }
    }


    struct SearchNode
    {
        int x = 0;
        int y = 0;
        int parentIndex = 0;
    };


    // Returns indices to tiles that are continuously at same level with tile at x, y
    static std::set<int> get_level_tiles(uint64_t* pWorld, int worldWidth, int x, int y)
    {
        // Iterate through adjacent tiles that are at same elevation by depth first search
        // TODO: optimize..

        // NOTE:
        // *This vector is used as "stack" so we use the last element kind of like our first
        std::vector<SearchNode> searchStack;
        // Indices of visited tiles
        std::set<int> visited;
        searchStack.push_back({ x, y, 0 });
        while (true)
        {
            if(searchStack.empty())
                break;

            SearchNode observe = searchStack.back();
            searchStack.pop_back();
            const int observeIndex = observe.x + observe.y * worldWidth;

            if (visited.find(observeIndex) != visited.end())
                continue;

            SearchNode adjacentNodes[4] =
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
                        searchStack.push_back(adjacentNodes[i]);
                    else if (adjacentHeight < currentHeight)
                        return get_level_tiles(pWorld, worldWidth, adjacentNodes[i].x, adjacentNodes[i].y);
                        //leakInto[adjacentIndex] = adjacentNodes[i];
                }
            }

            visited.insert(observeIndex);
        }
        return visited;
    }


    static void generate_world_waters_TEST(uint64_t* pWorld, int worldWidth)
    {
        int testStartX = 16;
        int testStartY = 61;

        std::set<int> finalWaters;
        const int iterCount = 1024;
        for (int i = 0; i < iterCount; ++i)
        {
            int rx = std::rand() % worldWidth;
            int ry = std::rand() % worldWidth;

            std::set<int> waterTiles = get_level_tiles(
                pWorld,
                worldWidth,
                rx,
                ry
            );
            for (int w : waterTiles)
                finalWaters.insert(w);
        }

        /*
        int iterCount = 0;
        std::set<int> finalWaters;
        if (leaks.empty())
        {
            finalWaters = waterTiles;
        }
        else
        {
            while (!leaks.empty())
            {
                std::unordered_map<int, SearchNode>::const_iterator it;
                std::set<int> leaksToErase;
                std::unordered_map<int, SearchNode> leaksToAdd;
                for (it = leaks.begin(); it != leaks.end(); ++it)
                {
                    const SearchNode& n = it->second;
                    std::unordered_map<int, SearchNode> newLeaks;
                    std::set<int> newWaterTiles = get_level_tiles(
                        pWorld,
                        worldWidth,
                        n.x,
                        n.y,
                        newLeaks
                    );
                    if (newLeaks.empty())
                    {
                        leaksToErase.insert(it->first);
                        for (int t : newWaterTiles)
                            finalWaters.insert(t);
                    }
                    else
                    {
                        for (std::unordered_map<int, SearchNode>::const_iterator nit = newLeaks.begin(); nit != newLeaks.end(); ++nit)
                            leaksToAdd[nit->first] = nit->second;
                    }
                    Debug::log("Iter: " + std::to_string(iterCount) + " leak count: " + std::to_string(leaks.size()) + " leaks to erase: " + std::to_string(leaksToErase.size()) + " final waters: " + std::to_string(finalWaters.size()));
                    ++iterCount;
                }
                Debug::log("___TEST___READY iterations: " + std::to_string(iterCount));
                for (int l : leaksToErase)
                    leaks.erase(l);
                for (std::pair<int, SearchNode> n : leaksToAdd)
                    leaks[n.first] = n.second;
            }
        }
        */

        Debug::log("___TEST___final water count = " + std::to_string(finalWaters.size()));

        for (const int i : finalWaters)
        {
            set_tile_terrtype(*(pWorld + i), 1);
        }
    }


    static void generate_temperature_effect(uint64_t* pWorld, int worldWidth, int equatorYPos, int baseTemperature)
    {
        for(int y = 0; y < worldWidth; ++y)
        {
            for(int x = 0; x < worldWidth; ++x)
            {
                size_t index = x + y * worldWidth;
                int tileElevation = (int)get_tile_terrelevation(pWorld[index]);
                int distFromEq = std::abs(y - equatorYPos);

                int elevationMod = tileElevation;
                int eqDistMod = (distFromEq / 20);
                int temperatureScore = baseTemperature - elevationMod - eqDistMod;
                if(temperatureScore <= 0)
                    set_tile_terrtype(pWorld[index], 2);
            }
        }
    }

    void generate_world(uint64_t* pWorld, int worldWidth, int maxElevationVal, unsigned int seed, int equatorYPos, int baseTemperature)
    {
        //generate_world_elevation(pWorld, worldWidth, maxElevationVal, seed);
        generate_world_elevation_TEST(pWorld, worldWidth, maxElevationVal, seed);
        generate_world_erosion(pWorld, worldWidth, 4);
        generate_world_waters_TEST(pWorld, worldWidth);
        //generate_world_waters(pWorld, worldWidth);
        //generate_temperature_effect(pWorld, worldWidth, equatorYPos, baseTemperature);
    }
}
