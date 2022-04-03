
#include "Tile.h"
#include <cstring>

#include "BitShit.hpp"

namespace world
{
	
	void set_tile_uid(uint64_t& tile, uint32_t uid)
	{
		memcpy(&tile, &uid, sizeof(uint32_t));
	}

	
	void set_tile_geoinfo(uint64_t& tile, PK_byte value)
	{
		bitshit::set_area<uint64_t, PK_byte>(
				tile, value,
				TILE_STATE_POS_properties + TILE_STATE_POS_geoInfo,
				TILE_STATE_SIZE_geoInfo
				);
	}
	
	uint32_t get_tile_uid(uint64_t tile)
	{
		return (uint32_t)tile;
	}
}
