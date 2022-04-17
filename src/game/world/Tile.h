#pragma once

#include <cstdint>
#include "Common.h"

// Size of the full state (in bytes)
#define TILE_STATE_SIZE_BYTES 		8

// Sizes of different portions (exact size in BITS NOT BYTES!)
#define TILE_STATE_SIZE_uid		32
#define TILE_STATE_SIZE_properties	32

#define TILE_STATE_SIZE_terrElevation	3
#define TILE_STATE_SIZE_terrType	4
#define TILE_STATE_SIZE_thingCategory	3
#define TILE_STATE_SIZE_thingID		8
#define TILE_STATE_SIZE_action		3
#define TILE_STATE_SIZE_dir		3
#define TILE_STATE_SIZE_customVar	8


// Bit positions in the "state"
#define TILE_STATE_POS_properties	32

#define TILE_STATE_POS_terrElevation	0
#define TILE_STATE_POS_terrType		3
#define TILE_STATE_POS_thingCategory	7
#define TILE_STATE_POS_thingID		10
#define TILE_STATE_POS_action		18
#define TILE_STATE_POS_dir		21
#define TILE_STATE_POS_customVar	24

namespace world
{
	enum TileStateTerrElevation
	{
		TILE_STATE_terrElevAbyss = 		0,
		TILE_STATE_terrElevMid1 = 		1,
		TILE_STATE_terrElevMid2 = 		2,
		TILE_STATE_terrElevHighland = 		3,
		TILE_STATE_terrElevHills = 		4,
		TILE_STATE_terrElevMountains = 		5,
		TILE_STATE_terrElevHighMountains = 	6
	};


	enum TileStateTerrType
	{
		TILE_STATE_terrTypeCommonWater = 	0,
		TILE_STATE_terrTypeCommonDeadland = 	1,
		TILE_STATE_terrTypeCommonWasteland = 	2,
		TILE_STATE_terrTypeCommonRocky = 	3,
		TILE_STATE_terrTypeCommonSnow = 	4,
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
	
	
	void set_tile_uid		(uint64_t& tile, uint32_t uid);
	void set_tile_terrelevation	(uint64_t& tile, PK_ubyte value);
	void set_tile_terrtype		(uint64_t& tile, PK_ubyte value);
	void set_tile_thingcategory	(uint64_t& tile, PK_ubyte value);
	void set_tile_thingid		(uint64_t& tile, PK_ubyte value);
	void set_tile_action		(uint64_t& tile, PK_ubyte value);
	void set_tile_facingdir		(uint64_t& tile, PK_ubyte value);
	void set_tile_customvar		(uint64_t& tile, PK_ubyte value);


	uint32_t get_tile_uid		(uint64_t tile);
	PK_ubyte get_tile_terrelevation	(uint64_t tile);
	PK_ubyte get_tile_terrtype	(uint64_t tile);
	PK_ubyte get_tile_thingcategory	(uint64_t tile);
	PK_ubyte get_tile_thingid	(uint64_t tile);
	PK_ubyte get_tile_action	(uint64_t tile);
	PK_ubyte get_tile_facingdir	(uint64_t tile);
	PK_ubyte get_tile_customvar	(uint64_t tile);
}



