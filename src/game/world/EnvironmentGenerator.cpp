#include "EnvironmentGenerator.h"
#include "Debug.h"
#include "../../../Proj8315Common/src/Common.h"
#include "../../../Proj8315Common/src/Tile.h"
#include "game/Game.h"
#include <unordered_set>
#include <cmath>


using namespace gamecommon;


/*

Okay... this may require a little introduction how this works!


Fertile areas are created as follows:

1. We create initial initial "FertilityWeights"
These consists of water tiles. The values of the weights depends on
how many contiguous water tiles forms a body of water, like figure below:

w = water tile

0 0 0 0 0 0
0 0 0 0 0 0
0 0 w w 0 0
0 0 w w 0 0
0 0 0 0 0 0
0 0 0 0 0 0

2. After this we iterate areas outside of the water areas in circular pattern
and determine how "fertile" the tile should be depending on the inner ring - 1, which at
first iteration consists of the water tiles(which gets their fertility value from how
big body of water it is)

Then we go to next "ring" and always decrease current fertility value from inner ring which could
result in following pattern:

1 1 1 1 1 1
1 2 2 2 2 1
1 2 3 3 2 1
1 2 3 3 2 1
1 2 2 2 2 1
1 1 1 1 1 1

Yea... its like influence map, but with extra steps and written like shit...

TODO: Maybe rework the whole world generation code...

*/


namespace world
{
    using namespace objects;

    // NOTE: Fucking dumb way to do this atm. Just randomly put some plant or not...
    // TODO: Take possible plant life density into account using fertility score somehow...
    static void attempt_plant_environment_object(
        ObjectManager* pObjManager,
        TileStateTerrType terrainType,
        TileStateTemperature temperature,
        int tileIndex,
        int worldWidth,
        const float fertilityScore
    )
    {
        int diceThrow = std::rand() % 20;

        int requiredScore = 16;

        // NOTE: Not sure is it good idea to spawn env objects as Neutral faction...
        // TODO: Also way to access factions by indices and ENSURE THAT Neutral IS AT 0
        Faction* neutralFaction = Game::get()->accessFaction("Neutral");

        if (diceThrow >= requiredScore)
        {
            // NOTE: ATM JUST TESTING
            // TODO: Different env obj types for different environments!
            int x = tileIndex % worldWidth;
            int y = tileIndex / worldWidth;
            pObjManager->spawnObject(x, y, 1, *neutralFaction);
        }
    }


    void generate_environment(
        uint64_t* pWorld,
        int worldWidth,
        const std::unordered_map<int, int>& waterPoolSizes,
        ObjectManager* pObjManager
    )
    {
        if (!pObjManager)
        {
            Debug::log(
                "@world::generate_environment "
                "pObjManager was nullptr!",
                Debug::MessageType::FATAL_ERROR
            );
            // TODO: some assert system...
            return;
        }

        // key = tile index, value = fertility value
        static std::unordered_map<int, float> fertilityWeights;

        std::unordered_set<int> tilesToCheck;

        // Generate initial fertility weights from water tiles
        for (int y = 0; y < worldWidth; ++y)
        {
            for (int x = 0; x < worldWidth; ++x)
            {
                const int tileIndex = x + y * worldWidth;
                GC_ubyte tileType = get_tile_terrtype((uint64_t)*(pWorld + tileIndex));

                if (tileType == TileStateTerrType::TILE_STATE_terrTypeWater)
                {
                    fertilityWeights[tileIndex] = 0;

                    for (int y2 = y - 1; y2 <= y + 1; ++y2)
                    {
                        for (int x2 = x - 1; x2 <= x + 1; ++x2)
                        {
                            const int adjacentIndex = x2 + y2 * worldWidth;
                            // NOTE: atm purposefully adding the current tile here and not skipping...
                            // May want to test out if skipping the current...
                            if (adjacentIndex < 0 || adjacentIndex >= worldWidth * worldWidth)
                                continue;

                            uint64_t& adjacentRef = (uint64_t&)*(pWorld + adjacentIndex);
                            GC_ubyte adjacentType = get_tile_terrtype(adjacentRef);
                            if (adjacentType == TileStateTerrType::TILE_STATE_terrTypeWater)
                            {
                                // NOTE: Not sure what this multiplier should be in the end...
                                // just toying around atm...
                                const float poolMultiplier = 0.1f;
                                fertilityWeights[tileIndex] = waterPoolSizes.at(tileIndex) * poolMultiplier;
                            }
                            else
                            {
                                tilesToCheck.insert(adjacentIndex);
                            }
                        }
                    }
                }
            }
        }

        // iterate by "waves" starting from the first adjacent tiles to the water
        // and go further away
        std::unordered_set<int> checkedTiles;

        // How much temperature affects fertility further away from water source.
        // Like.. super hot areas are more drought far from water sources...
        std::unordered_map<TileStateTemperature, float> temperatureNegEffectMul =
        {
            { TILE_STATE_mild,      1 },
            { TILE_STATE_chilly,    1 },
            { TILE_STATE_cold,      1 },
            { TILE_STATE_freezing,  1 },
            { TILE_STATE_warm,      1 },
            { TILE_STATE_hot,       3 },
            { TILE_STATE_burning,   5 }
        };

        float maxFertScore = 0.0f;

        // Add to fertility values on first wave iteration
        // -> decrease the rest of the waves
        bool first = true;
        while (!tilesToCheck.empty())
        {
            std::unordered_set<int> newWaveTiles;
            std::unordered_set<int> toRemoveIndices;

            for (int tileIndex : tilesToCheck)
            {
                int adjacentTileIndices[8] = {
                    tileIndex - worldWidth, // n
                    tileIndex - worldWidth + 1, // ne
                    tileIndex + 1, // e
                    tileIndex + worldWidth + 1, // se
                    tileIndex + worldWidth, // s
                    tileIndex + worldWidth - 1, // sw
                    tileIndex - 1, // w
                    tileIndex - worldWidth - 1 // ne
                };

                float score = 0;
                float count = 0;

                std::unordered_set<int> potentialNewTiles;

                // Determine the "fertility value"
                for (int i = 0; i < 8; ++i)
                {
                    int adjacentTileIndex = adjacentTileIndices[i];
                    if (adjacentTileIndex < 0 || adjacentTileIndex >= worldWidth * worldWidth)
                        continue;

                    if (tilesToCheck.find(adjacentTileIndex) != tilesToCheck.end())
                        continue;

                    uint64_t& adjacentTileRef = *(pWorld + adjacentTileIndex);
                    if (fertilityWeights.find(adjacentTileIndex) == fertilityWeights.end())
                    {
                        // Check is this adjacent non water tile, we should add
                        if (get_tile_terrtype(adjacentTileRef) != TileStateTerrType::TILE_STATE_terrTypeWater)
                        {
                            if (tilesToCheck.find(adjacentTileIndex) == tilesToCheck.end())
                                potentialNewTiles.insert(adjacentTileIndex);
                        }

                        continue;
                    }
                    float deg = 1.0f;
                    if (first)
                        deg = 0.0f;

                    TileStateDirection iDir = (TileStateDirection)i;
                    if (iDir == TileStateDirection::TILE_STATE_dirNE ||
                        iDir == TileStateDirection::TILE_STATE_dirSE ||
                        iDir == TileStateDirection::TILE_STATE_dirSW ||
                        iDir == TileStateDirection::TILE_STATE_dirNW
                        )
                    {
                        deg = 2.0f;
                    }

                    TileStateTemperature adjacentTemperature = (TileStateTemperature)get_tile_temperature(adjacentTileRef);

                    score += fertilityWeights[adjacentTileIndex] - deg * temperatureNegEffectMul[adjacentTemperature];
                    count += 1.0f;
                }

                const float finalScore = score / count;
                // After figuring out the total fertility value -> figure out what to do...
                if (finalScore > 0)
                {
                    maxFertScore = std::max(finalScore, maxFertScore);
                    fertilityWeights[tileIndex] = finalScore;
                    newWaveTiles.insert(potentialNewTiles.begin(), potentialNewTiles.end());

                    uint64_t& tileRef = *(pWorld + tileIndex);
                    TileStateTerrType tileType = (TileStateTerrType)get_tile_terrtype(tileRef);
                    // Set tile to "fertile" if its not water or rock(atm rock indicates cliff)
                    if (tileType != TileStateTerrType::TILE_STATE_terrTypeWater && tileType != TileStateTerrType::TILE_STATE_terrTypeRock)
                    {
                        set_tile_terrtype(tileRef, TileStateTerrType::TILE_STATE_terrTypeFertile);

                        // NOTE JUST TESTING -> spawn tree depending on fertility score
                        TileStateTerrType tileType = (TileStateTerrType)get_tile_terrtype(tileRef);
                        TileStateTemperature temperature = (TileStateTemperature)get_tile_temperature(tileRef);
                        attempt_plant_environment_object(
                            pObjManager,
                            tileType,
                            temperature,
                            tileIndex,
                            worldWidth,
                            finalScore
                        );

                    }
                }

                toRemoveIndices.insert(tileIndex);
            }

            tilesToCheck.insert(newWaveTiles.begin(), newWaveTiles.end());
            for (int index : toRemoveIndices)
                tilesToCheck.erase(index);

            first = false;

            Debug::log("___TEST___MAX FERT SCORE: " + std::to_string(maxFertScore));
        }
    }
}
