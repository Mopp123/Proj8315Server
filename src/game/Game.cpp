
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
	const std::string neutralFactionName = "Neutral";
	Faction* neutralFaction = new Faction(neutralFactionName.c_str(), neutralFactionName.size());
	_factions.insert(std::make_pair(neutralFaction->getName(), neutralFaction));

	// initialize object types "library"
	_objectInfo = world::objects::load_obj_info_file("data/ObjectsConfig.txt");

	// Testing movement with these objs
	for (int i = 0; i < TEST_unitsCount; ++i)
	{
		int randX = std::rand() % _worldWidth;
		int randY = std::rand() % _worldWidth;
		_objUpdater->spawnObject(randX, randY, 2, neutralFaction);
	}
	
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
				if (diceThrow > 40)
					_objUpdater->spawnObject(x, y, 1, neutralFaction);
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
	std::vector<world::objects::ObjectInstanceData*> testUnits;
	for (int i = 0; i < TEST_unitsCount; ++i)
		testUnits.push_back(_objUpdater->accessObject(i));

	while(_run)
	{
		std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();
		
		// Test updating some random actions for some units..
		for (world::objects::ObjectInstanceData* obj : testUnits)
		{
			if (obj->getActionQueue().size() < 2)
			{
				int r = (std::rand() % 8) + 1;
				obj->addAction(r);
			}
		}

		_objUpdater->update();

		std::chrono::time_point<std::chrono::high_resolution_clock> endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> delta = endTime - startTime ;
		_deltaTime = delta.count();
		//Debug::log("Delta: " + std::to_string(_deltaTime));
	}
}

Message Game::addFaction(const char* factionName, size_t nameLen)
{
	std::lock_guard<std::mutex> lock(_mutex_faction);
	std::string responseMessage;
	
	std::string factionNameStr = factionName;
	Debug::log("Attempting to create faction: " + factionNameStr);

	auto iter = _factions.find(factionNameStr);
	if(iter == _factions.end())
	{
		_factions.insert(std::make_pair(factionNameStr, new Faction(factionName, nameLen)));
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
	size_t bufSize = sizeof(int32_t) + (observeRectWidth * observeRectWidth) * sizeof(uint64_t);
	
	PK_byte* buffer = new PK_byte[bufSize];
	memset(buffer, 0, bufSize);

	// Set first 4 bytes to contain message type name
	const int32_t messageType = MESSAGE_TYPE__GetWorldState;
	memcpy(buffer, (const void*)&messageType, sizeof(int32_t));

	size_t bufPos = sizeof(int32_t);
	
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
	const size_t objSize = _objectInfo[0].getNetwSize();
	const size_t totalBufSize = _objectInfo.size() * objSize;
	
	PK_byte* buffer = new PK_byte[totalBufSize];
	memset(buffer, 0, totalBufSize);

	size_t bufPos = 0;
	
	for(const world::objects::ObjectInfo& objInfo : _objectInfo)
	{
		// *NOTE! Not sure if needing to lock to prevent nothing funny happening...
		// get name
		memcpy(
			(void*)(buffer + bufPos), 
			(const void*)(&objInfo.name), 
			OBJECT_DATA_STRLEN_NAME
		);
		// get desc
		memcpy(
			(void*)(buffer + bufPos + OBJECT_DATA_STRLEN_NAME), 
			(const void*)(&objInfo.description), 
			OBJECT_DATA_STRLEN_DESCRIPTION
		);
		// get action slots
		for (int i = 0; i < TILE_STATE_MAX_action + 1; ++i)
		{
			memcpy(
				(void*)(buffer + 
					bufPos + 
					OBJECT_DATA_STRLEN_DESCRIPTION + 
					(i * OBJECT_DATA_STRLEN_ACTION_NAME)
				), 
				(const void*)(&objInfo.actionSlot[i]), 
				OBJECT_DATA_STRLEN_ACTION_NAME
			);
		}
		const size_t bufPosBeginStats =
			bufPos + 
			OBJECT_DATA_STRLEN_NAME +
			OBJECT_DATA_STRLEN_DESCRIPTION +
			((TILE_STATE_MAX_action + 1) * OBJECT_DATA_STRLEN_ACTION_NAME
		);
		// get stats
		// NOTE: At the moment all stats has to be single unsigned bytes
		memcpy(
			(void*)(buffer + bufPosBeginStats),
			(const void*)(&objInfo.speed),
			1
		);
		bufPos += objSize;
	}
	Message response(NULL_CLIENT, buffer, totalBufSize);
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

