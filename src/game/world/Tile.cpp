
#include "Tile.h"
#include <cstring>

namespace world
{
	
	void set_tile_uid(void* tile, uint32_t uid)
	{
		memcpy(tile, &uid, sizeof(uint32_t));
	}

	
	void set_tile_geoinfo(void* tile, PK_byte value)
	{

	}
	
	uint32_t get_tile_uid(void* tile)
	{
		return *((uint32_t*)tile);
	}
}
