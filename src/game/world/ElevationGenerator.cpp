#include "ElevationGenerator.h"
#include "../../../Proj8315Common/src/Common.h"
#include "../../../Proj8315Common/src/Tile.h"
#include "../../utils/Algorithm.h"
#include <cmath>


using namespace gamecommon;


namespace world
{
    void create_ramp(
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
                    // Not sure what's the point of below...?
                    //set_tile_terrtype(targetTile, TileStateTerrType::TILE_STATE_terrTypeFertile);
                }
            }
        }
    }

    std::vector<float> create_heightmap(
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

    void generate_world_elevation(uint64_t* pWorld, int worldWidth, int maxElevationVal, unsigned int seed)
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
                // Not sure what's the point below...?
                //set_tile_terrtype(pWorld[index], TileStateTerrType::TILE_STATE_terrTypeFertile);
            }
        }
    }


    // Generates cliffs and ramps
    void generate_world_cliffs(uint64_t* pWorld, int worldWidth, int cliffThreshold)
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
                            if (adjElevation - currentElevation >= cliffThreshold)
                            {
                                // Set to rock type to indicate steep cliff
                                set_tile_terrtype(
                                    pWorld[currentIndex],
                                    TileStateTerrType::TILE_STATE_terrTypeRock
                                );
                                set_tile_terrtype(
                                    pWorld[adjIndex],
                                    TileStateTerrType::TILE_STATE_terrTypeRock
                                );

                                // For every discovered cliff -> randomize do we put ramp there or not
                                int diceThrow1 = std::rand() % 20;
                                int diceThrow2 = std::rand() % 5;
                                if (diceThrow1 == 1)
                                {
                                    create_ramp(
                                        pWorld,
                                        worldWidth,
                                        x2,
                                        y2,
                                        diceThrow2 + 1
                                    );
                                    // Atm set ramp to dirt to have some indication that its ramp and not cliff
                                    set_tile_terrtype(
                                        pWorld[currentIndex],
                                        TileStateTerrType::TILE_STATE_terrTypeDirt
                                    );
                                    set_tile_terrtype(
                                        pWorld[adjIndex],
                                        TileStateTerrType::TILE_STATE_terrTypeDirt
                                    );
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
