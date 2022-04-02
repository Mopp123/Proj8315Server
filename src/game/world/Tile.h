#pragma once

#include <cstdint>
#include "Common.h"

// Size of the full state (in bytes)
#define TILE_STATE_SIZE_BYTES 		8

// Sizes of different portions (exact size in BITS NOT BYTES!)
#define TILE_STATE_SIZE_uid		32
#define TILE_STATE_SIZE_properties	32

#define TILE_STATE_SIZE_geoInfo		5
#define TILE_STATE_SIZE_effect		2
#define TILE_STATE_SIZE_thingCategory	3
#define TILE_STATE_SIZE_thingID		8
#define TILE_STATE_SIZE_action		3
#define TILE_STATE_SIZE_dir		3
#define TILE_STATE_SIZE_customVar	8


// Bit positions in the "state"
#define TILE_STATE_POS_properties	32

#define TILE_STATE_POS_geoInfo		0
#define TILE_STATE_POS_effect		5
#define TILE_STATE_POS_thingCategory	7
#define TILE_STATE_POS_thingID		10
#define TILE_STATE_POS_action		18
#define TILE_STATE_POS_dir		21
#define TILE_STATE_POS_customVar	24

namespace world
{
	enum TileStateEffect
	{
		TILE_STATE_effectNone = 	0,
		TILE_STATE_effectFire = 	1,
		TILE_STATE_effectPoison = 	2,
		TILE_STATE_effectPENDING = 	3 // Not decided yet, what this last usable val would be?
	};
	
	enum TileStateAction
	{
		TILE_STATE_actionIdle = 	0,
		TILE_STATE_actionMove = 	1,
		TILE_STATE_actionClassAction1 = 2,
		TILE_STATE_actionClassAction2 = 3,

		// Not decided yet, what would these be? ( these are also available values to use..)
		TILE_STATE_actionPENDING1 = 4,
		TILE_STATE_actionPENDING2 = 5,
		TILE_STATE_actionPENDING3 = 6,
		TILE_STATE_actionPENDING4 = 7,
	};

	struct Tile
	{
	
		uint64_t state = 0;
	};
	
	
	void set_tile_uid(void* tile, uint32_t uid);
	void set_tile_geoinfo(void* tile, PK_byte value);

	uint32_t get_tile_uid(void* tile);
}



