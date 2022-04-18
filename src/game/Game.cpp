
#include "Game.h"
#include "RequestHandler.h"
#include <cstring>
#include <mutex>

#include "game/stateUpdates/GeoUpdater.h"
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


	// Create world state updaters...
	_pGeoUpdater = new world::GeoUpdater(*this, 0.01f);
}

Game::~Game()
{
	delete _pGeoUpdater;
	delete[] _pWorld;
}


void Game::run()
{
	while(_run)
	{
		_pGeoUpdater->update();
	}
	/*
	std::lock_guard<std::mutex> lock(_mutex_worldState);
	world::set_tile_terrinfo(_pWorld[0 + 0 * GAME_WORLD_WIDTH], (PK_ubyte)(s_TEST_anim));
	s_TEST_anim += 0.25f;
	if(s_TEST_anim>=30.0f)
		s_TEST_anim = 0.0f;
		*/
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


Response Game::getWorldState(int xPos, int zPos, int observeRadius)
{
	const int observeRectWidth = (observeRadius * 2) + 1;
	size_t bufSize = (observeRectWidth * observeRectWidth) * sizeof(uint64_t);
	
	PK_byte* buffer = new PK_byte[bufSize];
	memset(buffer, 0, bufSize);

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

uint64_t Game::getTileState(int xPos, int zPos)
{
	if(validCoords(xPos, zPos))
	{
		// Not sure is this kind of "just observing" requiring locking...
		std::lock_guard<std::mutex> lock(_mutex_worldState);
		return _pWorld[xPos + zPos * GAME_WORLD_WIDTH];
	}
	return 0;
}
uint64_t Game::getTileState(int index)
{
	if(index >= 0 && index < GAME_WORLD_WIDTH * GAME_WORLD_WIDTH)
	{
		// Not sure is this kind of "just observing" requiring locking...
		std::lock_guard<std::mutex> lock(_mutex_worldState);
		return _pWorld[index];
	}
	return 0;
}

void Game::setTileState(int xPos, int zPos, uint64_t newState)
{
	if(validCoords(xPos, zPos))
	{
		std::lock_guard<std::mutex> lock(_mutex_worldState);
		_pWorld[xPos + zPos * GAME_WORLD_WIDTH] = newState;
	}	
}
void Game::setTileState(int index, uint64_t newState)
{
	if(index >= 0 && index < GAME_WORLD_WIDTH * GAME_WORLD_WIDTH)
	{
		std::lock_guard<std::mutex> lock(_mutex_worldState);
		_pWorld[index] = newState;
	}	
}

Game* Game::get()
{
	return s_pInstance;
}
