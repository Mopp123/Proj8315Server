#pragma once

#include <cstdint>


namespace world
{
    void generate_temperature_effect(
        uint64_t* pWorld,
        int worldWidth,
        int equatorYPos,
        int baseTemperature
    );
}
