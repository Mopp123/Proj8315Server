#include "TemperatureGenerator.h"
#include "../../../Proj8315Common/src/Common.h"
#include "../../../Proj8315Common/src/Tile.h"
#include <unordered_map>


using namespace gamecommon;


namespace world
{
    void generate_temperature_effect(
        uint64_t* pWorld,
        int worldWidth,
        int equatorYPos,
        int baseTemperature
    )
    {
        std::unordered_map<int, TileStateTemperature> tempMapping =
        {
            { 0, TileStateTemperature::TILE_STATE_freezing },
            { 1, TileStateTemperature::TILE_STATE_cold },
            { 2, TileStateTemperature::TILE_STATE_chilly },
            { 3, TileStateTemperature::TILE_STATE_mild },
            { 4, TileStateTemperature::TILE_STATE_warm },
            { 5, TileStateTemperature::TILE_STATE_hot },
            { 6, TileStateTemperature::TILE_STATE_burning }
        };

        for(int y = 0; y < worldWidth; ++y)
        {
            for(int x = 0; x < worldWidth; ++x)
            {
                size_t index = x + y * worldWidth;
                float fTileElevation = (int)get_tile_terrelevation(pWorld[index]);
                float distFromEq = (float)std::abs(y - equatorYPos);

                float distScore = std::min((distFromEq / (49.0f)), 6.0f);
                float elevationScore = std::max((fTileElevation / 6.0f), 0.0f);

                const int testBaseTemperature = 6;
                int totalTemperatureScore = std::max(testBaseTemperature - (int)((distScore + elevationScore) * 0.5f), 0);

                TileStateTemperature useTemperature = tempMapping[totalTemperatureScore];

                uint64_t& tileRef = *(pWorld + index);
                set_tile_temperature(tileRef, useTemperature);

                // NOTE: TESTING BELOW...
                // Alter tile terrain type depending on temperature
                GC_ubyte currentType = get_tile_terrtype(tileRef);
                // set non fertile if burning hot and no water around
                int nearWaterCount = 0;
                const int checkRadius = 3;
                if (useTemperature == TileStateTemperature::TILE_STATE_burning && currentType != TileStateTerrType::TILE_STATE_terrTypeRock)
                {
                    for (int y2 = y - checkRadius; y2 < y + checkRadius; ++y2)
                    {
                        for (int x2 = x - checkRadius; x2 < x + checkRadius; ++x2)
                        {
                            const int adjacentIndex = x2 + y2 * worldWidth;
                            if (adjacentIndex >= 0 && adjacentIndex < worldWidth * worldWidth)
                            {
                                uint64_t& adjacentTileRef = *(pWorld + adjacentIndex);
                                if (get_tile_terrtype(adjacentTileRef) == TileStateTerrType::TILE_STATE_terrTypeWater)
                                    ++nearWaterCount;
                            }
                        }
                    }
                    if (nearWaterCount < 3)
                        set_tile_terrtype(tileRef, TileStateTerrType::TILE_STATE_terrTypeDunes);
                }
            }
        }
    }
}
