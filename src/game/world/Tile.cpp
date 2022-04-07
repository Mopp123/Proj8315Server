
#include "Tile.h"
#include <cstring>

#include "BitShit.hpp"

namespace world
{
	/*
	void set_tile_uid(uint64_t& tile, uint32_t uid)
	{
		memcpy(&tile, &uid, sizeof(uint32_t));
	}

	
	void set_tile_geoinfo(uint64_t& tile, PK_ubyte value)
	{
		bitshit::set_area<uint64_t, PK_ubyte>(
				tile, value,
				TILE_STATE_POS_properties + TILE_STATE_POS_geoInfo,
				TILE_STATE_SIZE_geoInfo
				);
	}
	
	uint32_t get_tile_uid(uint64_t tile)
	{
		return (uint32_t)tile;
	}*/


	void set_tile_uid(uint64_t& tile, uint32_t uid) 
	{

		memcpy(&tile, &uid, sizeof(uint32_t));
	}

        void set_tile_terrinfo(uint64_t& tile, PK_ubyte value) 
	{

		bitshit::set_area<uint64_t, PK_ubyte>(
			tile, value,
			TILE_STATE_POS_properties + TILE_STATE_POS_terrInfo,
			TILE_STATE_SIZE_terrInfo
		);
	}
        void set_tile_thingcategory(uint64_t& tile, PK_ubyte value) 
	{
		bitshit::set_area<uint64_t, PK_ubyte>(
			tile, value,
			TILE_STATE_POS_properties + TILE_STATE_POS_thingCategory,
			TILE_STATE_SIZE_thingCategory
		);
	}
        void set_tile_thingid(uint64_t& tile, PK_ubyte value) 
	{
		bitshit::set_area<uint64_t, PK_ubyte>(
			tile, value,
			TILE_STATE_POS_properties + TILE_STATE_POS_thingID,
			TILE_STATE_SIZE_thingID
		);
	}
        void set_tile_action(uint64_t& tile, PK_ubyte value) 
	{
		bitshit::set_area<uint64_t, PK_ubyte>(
			tile, value,
			TILE_STATE_POS_properties + TILE_STATE_POS_action,
			TILE_STATE_SIZE_action
		);
	}
        void set_tile_facingdir(uint64_t& tile, PK_ubyte value)
	{
		bitshit::set_area<uint64_t, PK_ubyte>(
			tile, value,
			TILE_STATE_POS_properties + TILE_STATE_POS_dir,
			TILE_STATE_SIZE_dir
		);
	}
        void set_tile_customvar         (uint64_t& tile, PK_ubyte value) 
	{
		bitshit::set_area<uint64_t, PK_ubyte>(
			tile, value,
			TILE_STATE_POS_properties + TILE_STATE_POS_customVar,
			TILE_STATE_SIZE_customVar
		);
	}


        uint32_t get_tile_uid(uint64_t tile) 
	{
		return (uint32_t)tile; // Should work since first 32 bits are uid?
	}
        PK_ubyte get_tile_terrinfo(uint64_t tile) 
	{
		PK_ubyte output = 0;
		bitshit::convert_area<uint64_t, PK_ubyte>(
			tile, output, 
			TILE_STATE_POS_properties + TILE_STATE_POS_terrInfo,
			TILE_STATE_SIZE_terrInfo
		);
		return output;
	}
        PK_ubyte get_tile_thingcategory(uint64_t tile) 
	{
		PK_ubyte output = 0;
		bitshit::convert_area<uint64_t, PK_ubyte>(
			tile, output, 
			TILE_STATE_POS_properties + TILE_STATE_POS_thingCategory,
			TILE_STATE_SIZE_thingCategory
		);
		return output;
	}
        PK_ubyte get_tile_thingid  (uint64_t tile) 
	{
		PK_ubyte output = 0;
		bitshit::convert_area<uint64_t, PK_ubyte>(
			tile, output, 
			TILE_STATE_POS_properties + TILE_STATE_POS_thingID,
			TILE_STATE_SIZE_thingID
		);
		return output;
	}
        PK_ubyte get_tile_action         (uint64_t tile) 
	{
		PK_ubyte output = 0;
		bitshit::convert_area<uint64_t, PK_ubyte>(
			tile, output, 
			TILE_STATE_POS_properties + TILE_STATE_POS_action,
			TILE_STATE_SIZE_action
		);
		return output;
	}
        PK_ubyte get_tile_facingdir      (uint64_t tile) 
	{
		PK_ubyte output = 0;
		bitshit::convert_area<uint64_t, PK_ubyte>(
			tile, output, 
			TILE_STATE_POS_properties + TILE_STATE_POS_dir,
			TILE_STATE_SIZE_dir
		);
		return output;
	}
        PK_ubyte get_tile_customvar      (uint64_t tile) 
	{
		PK_ubyte output = 0;
		bitshit::convert_area<uint64_t, PK_ubyte>(
			tile, output, 
			TILE_STATE_POS_properties + TILE_STATE_POS_customVar,
			TILE_STATE_SIZE_customVar
		);
		return output;
	}
}



