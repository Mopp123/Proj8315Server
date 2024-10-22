#include "WorldGenerator.h"
#include "../../../Proj8315Common/src/Common.h"
#include "../../../Proj8315Common/src/Tile.h"
#include "../../utils/Algorithm.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_set>
#include <vector>
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


    static void generate_world_erosion(uint64_t* pWorld, int worldWidth)
    {
        //const int cliffValThreshold = 8;
        const int cliffValThreshold = 12;
        for(int y = 0; y < worldWidth; ++y)
        {
            for(int x = 0; x < worldWidth; ++x)
            {
                const int check_radius = 2;
                int steepness = 1;
                int currentIndex = x + y * worldWidth;
                int currentElevation = (int)get_tile_terrelevation(pWorld[currentIndex]);
                bool isFlat = is_tile_flat(pWorld, worldWidth, x, y, currentElevation);
                for(int y2 = y - check_radius; y2 <= y + check_radius; ++y2)
                {
                    for(int x2 = x - check_radius; x2 <= x + check_radius; ++x2)
                    {
                        const int adjIndex = x2 + y2 * worldWidth;
                        if(adjIndex >= 0 && adjIndex < worldWidth * worldWidth)
                        {
                            int adjElevation = (int)get_tile_terrelevation(pWorld[adjIndex]);
                            steepness += std::abs(currentElevation - adjElevation);
                            if(adjElevation == currentElevation && steepness - 1 > 0 && isFlat)
                                steepness -= 1;
                        }
                    }
                }
                int diceThrow = std::rand() % steepness;
                if(diceThrow >= cliffValThreshold)
                {
                    set_tile_terrtype(pWorld[currentIndex], 3);
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
        generate_world_elevation(pWorld, worldWidth, maxElevationVal, seed);
        generate_world_erosion(pWorld, worldWidth);
        generate_world_waters(pWorld, worldWidth);
        generate_temperature_effect(pWorld, worldWidth, equatorYPos, baseTemperature);
    }
}
