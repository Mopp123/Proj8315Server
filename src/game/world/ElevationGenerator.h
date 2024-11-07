#pragma once

#include <cstdint>
#include <vector>


namespace world
{
    void create_ramp(
        uint64_t* pWorld,
        int worldWidth,
        int posX,
        int posY,
        int rampWidth
    );

    std::vector<float> create_heightmap(
        int width,
        unsigned int seed,
        int octaveCount,
        float scaleDivisor,
        int maxElevationVal
    );

    void generate_world_elevation(
        uint64_t* pWorld,
        int worldWidth,
        int maxElevationVal,
        unsigned int seed
    );

    // NOTE: This SHOULD be called after water generation so that
    // "water may fill pools nicely" otherwise all these "water pools"
    // are just half full:D ...I know this is fucking stupid system, but
    // had to compromise due to time constraints...
    // TODO: Maybe rework the whole world generation shit...
    void generate_world_cliffs(
        uint64_t* pWorld,
        int worldWidth,
        int cliffThreshold
    );
}
