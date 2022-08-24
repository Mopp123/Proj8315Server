#pragma once

#include <cstdint>

namespace world
{
	// *NOTE! atm we assume that maxElevation val is very small val(like max val of 3 bit unsigned int)
	void generate_world(uint64_t* pWorld, int worldWidth, int maxElevationVal, unsigned int seed, int equatorYPos, int baseTemperature);
}
