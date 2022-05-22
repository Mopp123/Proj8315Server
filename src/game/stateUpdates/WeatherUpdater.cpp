
#include "WeatherUpdater.h"
#include "game/Game.h"
#include "game/world/Tile.h"
#include "Debug.h"

#include <algorithm>

namespace world
{

	WeatherUpdater::WeatherUpdater(Game& gameRef, float updateCooldown, const std::unordered_set<int>& initialWaterTiles) :
		StateUpdater(gameRef, updateCooldown),
		_waterTiles(initialWaterTiles)
	{
		/*
		for(auto waterTile : _waterTiles)
		{
			uint64_t tileState = _gameRef.getTileState(waterTile);
			set_tile_terrtype(tileState, TileStateTerrType::TILE_STATE_terrTypeCommonWater);
			_gameRef.setTileState(waterTile, tileState);
		}*/

		Debug::log("WeatherUpdater initialized");
	}
	
	WeatherUpdater::~WeatherUpdater()
	{}

	void WeatherUpdater::updateFunc()
	{
		/*
		// This determines how aggressively we are spawning rain clouds
		const int cloudsPerCycleCount = 10;
		for(int i = 0; i < cloudsPerCycleCount; ++i)
		{

		// Pick random water tile which triggers raining
		int r = std::rand() % _waterTiles.size();

		auto randIter = _waterTiles.begin();
		std::advance(randIter, r);
		int toTriggerRainIndex = *randIter;
		
		int diceThrow_toRain = std::rand() % _dice_toRain;
		int toRainThreshold = 10;
		if(diceThrow_toRain >= toRainThreshold)
		{
			// Randomize near by tile where the rain hits
			int toRainIndex = toTriggerRainIndex + ((std::rand() % 10) - 5);
			toRainIndex += ((std::rand() % 3) - 1) * GAME_WORLD_WIDTH;
			// check is this a valid tile
			if(toRainIndex >= 0 && toRainIndex < GAME_WORLD_WIDTH * GAME_WORLD_WIDTH)
			{
				uint64_t tileState = _gameRef.getTileState(toRainIndex);
				// NOTE! Terr effect is flag. There may be multiple effects same time
				// -> need to take this into account!
				PK_ubyte currentFlags = get_tile_terreffect(tileState);
				// And finally MAKE IT RAAIN...
				set_tile_terreffect(tileState, currentFlags | TileStateTerrEffectFlags::TILE_STATE_terrEffectRain);
				
				_gameRef.setTileState(toRainIndex, tileState);

				if(_rainingTiles.find(toRainIndex) == _rainingTiles.end())
				{
					// check what type of tile this is, we are raining on?
					PK_ubyte toRainTileType = get_tile_terrtype(tileState);
					if(toRainTileType == TILE_STATE_terrTypeCommonWater)
						_rainingTiles[toRainIndex].first = 50;
					else
						_rainingTiles[toRainIndex].first = 1;

					_rainingTiles[toRainIndex].second = 100;
				}
			}
		}
		
		}
		
		// Check all raining tiles, do we want to stop raining or cause flooding
		for(auto it = _rainingTiles.begin(); it != _rainingTiles.end(); )
		{
			PK_ubyte& cloudMoisture = it->second.second;
			PK_ubyte& groundMoisture = it->second.first;
			
			int tileIndex = it->first;
			uint64_t tileState = _gameRef.getTileState(tileIndex);

			// decrease cloud moisture
			cloudMoisture--;
			// increase ground moisture
			groundMoisture++;
			
			// Flood to adjacent tiles, if enough ground moisture:D
			if(groundMoisture >= 120)
			{
				floodToAdjacents(tileIndex, tileState);
			}

			// Stop raining if "cloud out of moisture":D
			if(cloudMoisture <= 1)
			{
				PK_ubyte currentEffects = get_tile_terreffect(tileState);
				currentEffects = currentEffects ^ (PK_ubyte)TileStateTerrEffectFlags::TILE_STATE_terrEffectRain;
				
				set_tile_terreffect(tileState, currentEffects);
				_gameRef.setTileState(it->first, tileState);
				_rainingTiles.erase(it++);
			}
			else
			{
				++it;
			}
		}

		// Check water tiles, do we want to dry up..
		for(auto it = _waterTiles.begin(); it != _waterTiles.end(); )
		{

			bool remove = false;
			// If adjacent tiles lower -> dry this one
			
			int tileIndex = *it;
			uint64_t tileState = _gameRef.getTileState(tileIndex);	
			PK_ubyte elevation = get_tile_terrelevation(tileState);

			int adjacents[8];
			adjacents[0] = tileIndex - 1; //left
			adjacents[1] = tileIndex + 1; //right
			adjacents[2] = tileIndex - GAME_WORLD_WIDTH; //top
			adjacents[3] = tileIndex + GAME_WORLD_WIDTH; //bottom
			

			adjacents[4] = tileIndex - GAME_WORLD_WIDTH - 1; // top left
			adjacents[5] = tileIndex - GAME_WORLD_WIDTH + 1; // top right
			adjacents[6] = tileIndex + GAME_WORLD_WIDTH - 1; // bottom left
			adjacents[7] = tileIndex + GAME_WORLD_WIDTH + 1; // bottom right
			

			int surroundedByWater = 0;
			for(int i = 0; i < 8; ++i)
			{
				uint64_t adjacentState = _gameRef.getTileState(adjacents[i]);
				PK_ubyte adjacentTerrType = get_tile_terrtype(adjacentState);

				PK_ubyte adjacentElevation = get_tile_terrelevation(adjacentState);
				// Flooding is possible only if same or lower elevation
				if(adjacentElevation < elevation)
				{
					set_tile_terrtype(tileState, TILE_STATE_terrTypeCommonDeadland);
					_gameRef.setTileState(tileIndex, tileState);
					remove = true;
				}

				// remove from water tiles, if all adjacent water as well -> more rain on land
				if(adjacentTerrType == TileStateTerrType::TILE_STATE_terrTypeCommonWater)
					surroundedByWater++;
			}


			if(remove || surroundedByWater > 4)
			{
				if(remove)
				{
					set_tile_terrtype(tileState, TileStateTerrType::TILE_STATE_terrTypeCommonDeadland);
					_gameRef.setTileState(tileIndex, tileState);
				}
				_waterTiles.erase(it++);
			}
			else
			{
				it++;
			}
		}

	*/
	}

	void WeatherUpdater::floodToAdjacents(int tileIndex, uint64_t tileState)
	{
		/*
		PK_ubyte elevation = get_tile_terrelevation(tileState);

		int adjacents[8];
		adjacents[0] = tileIndex - 1; //left
		adjacents[1] = tileIndex + 1; //right
		adjacents[2] = tileIndex - GAME_WORLD_WIDTH; //top
		adjacents[3] = tileIndex + GAME_WORLD_WIDTH; //bottom
		

		adjacents[4] = tileIndex - GAME_WORLD_WIDTH - 1; // top left
		adjacents[5] = tileIndex - GAME_WORLD_WIDTH + 1; // top right
		adjacents[6] = tileIndex + GAME_WORLD_WIDTH - 1; // bottom left
		adjacents[7] = tileIndex + GAME_WORLD_WIDTH + 1; // bottom right
		
		for(int i = 0; i < 4; ++i)
		{
			uint64_t adjacentState = _gameRef.getTileState(adjacents[i]);
			PK_ubyte adjacentElevation = get_tile_terrelevation(adjacentState);
			// Flooding is possible only if same or lower elevation
			if(adjacentElevation <= elevation)
			{
				set_tile_terrtype(adjacentState, TILE_STATE_terrTypeCommonWater);
				_gameRef.setTileState(adjacents[i], adjacentState);
				_waterTiles.insert(adjacents[i]);
			}
		}
		*/
	}
}
