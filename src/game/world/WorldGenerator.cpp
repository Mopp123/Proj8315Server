#include "WorldGenerator.h"
#include "../../../Proj8315Common/src/Common.h"
#include "../../../Proj8315Common/src/Tile.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <unordered_map>
#include "../../Debug.h"

#include "ElevationGenerator.h"
#include "WaterGenerator.h"
#include "TemperatureGenerator.h"
#include "EnvironmentGenerator.h"


using namespace gamecommon;

namespace world
{
    using namespace objects;


    void generate_world(
        uint64_t* pWorld,
        int worldWidth,
        int maxElevationVal,
        unsigned int seed,
        int equatorYPos,
        int baseTemperature,
        objects::ObjectManager* pObjManager
    )
    {
        generate_world_elevation(pWorld, worldWidth, maxElevationVal, seed);
        std::unordered_map<int, int> waterPoolSizes = generate_world_waters(pWorld, worldWidth, 2048);
        generate_world_cliffs(pWorld, worldWidth, 4);
        generate_temperature_effect(pWorld, worldWidth, equatorYPos, baseTemperature);
        generate_environment(pWorld, worldWidth, waterPoolSizes, pObjManager);
    }
}
