
#include "Game.h"
#include "../Common.h"
#include "MessageHandler.h"
#include <cstring>
#include <mutex>
#include <unordered_set>

#include "objects/Object.h"
#include "objects/ObjectUpdater.h"
#include "world/Tile.h"
#include "world/WorldGenerator.h"
#include "../Debug.h"

#include <chrono>


Game* Game::s_pInstance = nullptr;

Game::Game(int worldWidth) :
	_worldWidth(worldWidth)
{
	s_pInstance = this;

	// Init world state
	_pWorld = new uint64_t[_worldWidth * _worldWidth];
	memset((void*)_pWorld, 0, sizeof(uint64_t) * _worldWidth * _worldWidth);
	
	unsigned int worldGenSeed = 4718;
	int maxElevationVal = 15; // max val of 3 bit uint
	world::generate_world(_pWorld, _worldWidth, maxElevationVal, worldGenSeed, 128, 20);

	_objUpdater = new world::objects::ObjectUpdater(*this);

	// *Initially we always need at least one "static-neutral" faction
	Faction* neutralFaction = new Faction("Neutral");
	_factions.insert(std::make_pair(neutralFaction->getName(), neutralFaction));

	// initialize object types "library"
	// *JUST FOR TESTING ATM!
	uint64_t testInitialObjState = 0;
	world::set_tile_thingid(testInitialObjState, 1);
	
	_objectInfo.push_back({"Empty", "", 0, 0});
	_objectInfo.push_back({"Tree1", "A testing tree object", 0, testInitialObjState});
	_objectInfo.push_back({"Movement Test", "For testing movement stuff", 1, testInitialObjState});

	for (int y = 0; y < _worldWidth; ++y)
	{
		for (int x = 0; x < _worldWidth; ++x)
		{
			int index = x + y * _worldWidth;
			uint64_t currentState = _pWorld[index];
			PK_ubyte terrType = world::get_tile_terrtype(currentState);
			if (terrType == 4)
			{
				int diceThrow = std::rand() % 100;
				if (diceThrow > 15)
				{
					_objUpdater->spawnObject(x, y, 1, neutralFaction);
				}
			}
		}
	}
}

Game::~Game()
{
	for (std::pair<std::string, Faction*> f : _factions)
		delete f.second;

	delete _objUpdater;
	delete[] _pWorld;
}

void Game::run()
{
	while(_run)
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();
		
		_objUpdater->update();

		std::chrono::time_point<std::chrono::high_resolution_clock> endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> delta = endTime - startTime ;
		//Debug::log("Game update took: " + std::to_string(delta.count()));
		_deltaTime = delta.count();

		//Debug::log("Delta: " + std::to_string(_deltaTime));
	}
}

Message Game::addFaction(const char* factionName)
{
	std::lock_guard<std::mutex> lock(_mutex_faction);
	std::string responseMessage;
	
	std::string factionNameStr = factionName;
	Debug::log("Attempting to create faction: " + factionNameStr);

	auto iter = _factions.find(factionNameStr);
	if(iter == _factions.end())
	{
		_factions.insert(std::make_pair(factionNameStr, new Faction(factionName)));
		responseMessage = "Success";
		Debug::log("New faction created successfully(faction count: " + std::to_string(_factions.size()));
	}
	else
	{
		responseMessage = "Failure";
	}

	return Message(NULL_CLIENT, responseMessage.data(), responseMessage.size());
}

Message Game::getWorldState(int xPos, int zPos, int observeRadius) const
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
	Message response(NULL_CLIENT, buffer, bufSize);
	delete[] buffer;
	return response;
}

Message Game::getObjectInfo() const
{
	const size_t objSize = _objectInfo[0].getSize();
	const size_t bufSize = _objectInfo.size() * objSize;
	
	PK_byte* buffer = new PK_byte[bufSize];
	memset(buffer, 0, bufSize);

	size_t bufPos = 0;
	
	for(const world::objects::ObjectInfo& objInfo : _objectInfo)
	{
		// *NOTE! Not sure if needing to lock to prevent nothing funny happening...
		memcpy((void*)(buffer + bufPos), (const void*)(&objInfo), objSize);
		bufPos += objSize;
	}
	Message response(NULL_CLIENT, buffer, bufSize);
	delete[] buffer;
	return response;
}

uint64_t Game::getTileState(int xPos, int zPos) const
{
	if(validCoords(xPos, zPos))
	{
		// Not sure is this kind of "just observing" requiring locking...
		std::lock_guard<std::mutex> lock(_mutex_worldState);
		return _pWorld[xPos + zPos * _worldWidth];
	}
	return 0;
}

uint64_t Game::getTileState(int index) const
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

float Game::getDeltaTime()
{
	return _deltaTime;
}

Game* Game::get()
{
	return s_pInstance;
}

