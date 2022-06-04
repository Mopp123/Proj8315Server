
#include "Game.h"
#include "Common.h"
#include "RequestHandler.h"
#include <cstring>
#include <mutex>
#include <unordered_set>

#include "world/Tile.h"
#include "world/WorldGenerator.h"
#include "Debug.h"

#include <chrono>

Game* Game::s_pInstance = nullptr;

Game::Game(int worldWidth) :
	_worldWidth(worldWidth)
{
	s_pInstance = this;

	// Init world state
	_pWorld = new uint64_t[_worldWidth * _worldWidth];
	memset((void*)_pWorld, 0, sizeof(uint64_t) * _worldWidth * _worldWidth);

	world::set_tile_terrtype(_pWorld[1 + 1 * _worldWidth], 1);
	world::set_tile_terrtype(_pWorld[2 + 1 * _worldWidth], 2);
	world::set_tile_terrtype(_pWorld[3 + 1 * _worldWidth], 3);
	world::set_tile_terrtype(_pWorld[4 + 1 * _worldWidth], 4);

	// Testing perlin noise world generation..
//	float* seedArr = new float[_worldWidth * _worldWidth];
//	for(int i = 0; i < _worldWidth * _worldWidth; ++i)
//		seedArr[i] = (float)(std::rand() % 150) * 0.01f;
//	
//	std::vector<float> noiseMap = generate_perlin2D(seedArr, _worldWidth, 4, 2.0f);
//	delete[] seedArr;
//
//	for(int i = 0; i < _worldWidth * _worldWidth; ++i)
//	{
//		float nVal = noiseMap[i] * 10.0f;
//		PK_ubyte height = (PK_ubyte)nVal;
//		Debug::log("h: " + std::to_string(nVal));
//		world::set_tile_terrelevation(_pWorld[i], height);
//	}
	unsigned int worldGenSeed = 4718;
	int maxElevationVal = 15; // max val of 3 bit uint
	world::generate_world(_pWorld, _worldWidth, maxElevationVal, worldGenSeed);
	world::generate_world_waters(_pWorld, _worldWidth);
	world::generate_temperature_effect(_pWorld, _worldWidth, 128, 20);
}

Game::~Game()
{
	delete[] _pWorld;
}


void Game::run()
{
	while(_run)
	{
		//std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();
		
		//_pGeoUpdater->update();
		//_pWeatherUpdater->update();


		/*std::chrono::time_point<std::chrono::high_resolution_clock> endTime = std::chrono::high_resolution_clock::now();

		std::chrono::duration<float> delta = endTime - startTime ;
		Debug::log("Game update took: " + std::to_string(delta.count()));*/
	}
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
			if(x >= 0 && x < _worldWidth && z >= 0 && z < _worldWidth)
			{
				// Should never go out of range? since prev if?.
				int tileIndex = x + z * _worldWidth;
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
		return _pWorld[xPos + zPos * _worldWidth];
	}
	return 0;
}
uint64_t Game::getTileState(int index)
{
	if(index >= 0 && index < _worldWidth * _worldWidth)
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
		_pWorld[xPos + zPos * _worldWidth] = newState;
	}	
}
void Game::setTileState(int index, uint64_t newState)
{
	if(index >= 0 && index < _worldWidth * _worldWidth)
	{
		std::lock_guard<std::mutex> lock(_mutex_worldState);
		_pWorld[index] = newState;
	}	
}

Game* Game::get()
{
	return s_pInstance;
}
