#pragma once

#include <mutex>
#include <cstdint>
#include <string>
#include <unordered_map>


#include "Responses.h"
#include "Faction.h"

#include "stateUpdates/GeoUpdater.h"
#include "stateUpdates/WeatherUpdater.h"

#define GAME_WORLD_WIDTH 2000

class Request;

class Game
{
private:

	mutable std::mutex _mutex_faction;
	mutable std::mutex _mutex_worldState;

	std::unordered_map<std::string, Faction> _factions;
	uint64_t* _pWorld = nullptr;

	static Game* s_pInstance;
	
	world::GeoUpdater* _pGeoUpdater = nullptr;
	world::WeatherUpdater* _pWeatherUpdater = nullptr;
	
	bool _run = true;

public:

	Game();
	Game(const Game&) = delete;
	~Game();
	
	void run();

	Response addFaction(const std::string& userID, const std::string& factionName);
	Response getWorldState(int xPos, int zPos, int observeRadius);
	
	uint64_t getTileState(int xPos, int zPos);
	uint64_t getTileState(int index);
	void setTileState(int xPos, int zPos, uint64_t newState);
	void setTileState(int index, uint64_t newState);

	static Game* get();

	inline bool validCoords(int x, int z) const 
	{
		int index = x + z * GAME_WORLD_WIDTH;
		return index >= 0 && index < GAME_WORLD_WIDTH * GAME_WORLD_WIDTH;
      	}
private:

};
