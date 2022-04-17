#pragma once

#include <mutex>
#include <cstdint>
#include <string>
#include <unordered_map>


#include "Responses.h"
#include "Faction.h"

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



public:

	Game();
	Game(const Game&) = delete;
	~Game();
	
	void update();

	Response addFaction(const std::string& userID, const std::string& factionName);
	Response getWorldState(int xPos, int zPos, int observeRadius);

	static Game* get();

private:

};
