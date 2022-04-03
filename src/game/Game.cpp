
#include "Game.h"
#include "RequestHandler.h"
#include <cstring>
#include <mutex>

#include "world/Tile.h"
#include "Debug.h"
#include <iostream>

Game* Game::s_pInstance = nullptr;

Game::Game()
{
	s_pInstance = this;

	// Init world state
	_pWorld = new uint64_t[GAME_WORLD_WIDTH * GAME_WORLD_WIDTH];
	memset((void*)_pWorld, 0, sizeof(uint64_t) * GAME_WORLD_WIDTH * GAME_WORLD_WIDTH);

	// Test setting some state into some tiles..
	world::set_tile_uid(_pWorld[1 + 1 * GAME_WORLD_WIDTH], 1);
	world::set_tile_uid(_pWorld[2 + 1 * GAME_WORLD_WIDTH], 2);
	world::set_tile_uid(_pWorld[1 + 2 * GAME_WORLD_WIDTH], 3);
	world::set_tile_uid(_pWorld[3 + 2 * GAME_WORLD_WIDTH], 4);
}

Game::~Game()
{

	delete[] _pWorld;
}

Response Game::addFaction(const std::string& userID, const std::string& factionName)
{
	std::lock_guard<std::mutex> lock(_mutex_faction);
	std::string responseMessage;
	
	Debug::log("Attempting to create faction: " + factionName + "(strlen: " + std::to_string(factionName.size()) + ")");

	auto iter = _factions.find(userID);
	if(iter == _factions.end())
	{
		Faction newFaction(factionName);
		_factions.insert(std::make_pair(userID, newFaction));
		responseMessage = "Success";

		Debug::log("New faction created successfully(faction count: " + std::to_string(_factions.size()));
	}
	else
	{
		responseMessage = "Failure";
	}

	return { responseMessage.data(), responseMessage.size() };
}


Response Game::getWorldState(int xPos, int zPos)
{
	const int observeRadius = 5;
	const int observeRectWidth = (observeRadius * 2) + 1;
	size_t bufSize = (observeRectWidth * observeRectWidth) * sizeof(uint64_t);
	
	PK_byte* buffer = new PK_byte[bufSize];
	size_t bufPos = 0;
	
	for(int z = zPos - observeRadius; z <= zPos + observeRadius; ++z)
	{
		for(int x = xPos - observeRadius; x <= xPos + observeRadius; ++x)
		{
			//Debug::log("bufPos: " + std::to_string(bufPos));
			// Make sure coords are valid tile coords
			if(x >= 0 && x < GAME_WORLD_WIDTH && z >= 0 && z <= GAME_WORLD_WIDTH)
			{
				// Should never go out of range? since prev if?.
				int tileIndex = x + z * GAME_WORLD_WIDTH;
				// .. need to lock so nothing funny happens...
				std::lock_guard<std::mutex> lock(_mutex_worldState);
				memcpy((void*)(buffer + bufPos), (void*)(_pWorld + tileIndex), sizeof(uint64_t));
				
			}
			bufPos += sizeof(uint64_t);

		}
	}
	Response response(buffer, bufSize);
	delete[] buffer;
	return response;
}

Game* Game::get()
{
	return s_pInstance;
}
