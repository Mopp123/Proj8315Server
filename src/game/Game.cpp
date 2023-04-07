#include "Game.h"
#include "../Common.h"
#include "Server.h"
#include "MessageHandler.h"

#include "objects/Object.h"
#include "objects/ObjectUpdater.h"
#include "world/Tile.h"
#include "world/WorldGenerator.h"
#include "../Debug.h"

#include <cstring>
#include <mutex>
#include <unordered_set>
#include <chrono>
#include <utility>
#include <string>


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
    PK_ubyte neutralDeployments[FACTION_MAX_DEPLOY_COUNT];
    memset(neutralDeployments, 2, FACTION_MAX_DEPLOY_COUNT);
    Faction* neutralFaction = new Faction(neutralFactionName.data(), neutralFactionName.size());
    neutralFaction->setDeployments(neutralDeployments, FACTION_MAX_DEPLOY_COUNT);
    neutralFaction->markUpdated(false);
    _factions.insert(std::make_pair(neutralFaction->getName(), neutralFaction));

    // initialize object types "library"
    _objectInfo = world::objects::load_obj_info_file("data/objects-conf.txt");
    // determine total size of the objLib in bytes
    _totalObjInfoSize = _objectInfo.size() * world::objects::get_netw_objinfo_size();
    _objInfoInitialized = true;

    // Testing movement with these objs
    for (int i = 0; i < 1000; ++i)
    {
    	int randX = std::rand() % _worldWidth;
    	int randY = std::rand() % _worldWidth;
    	if (_objUpdater->spawnObject(randX, randY, 2, neutralFaction))
    		_testUnits.push_back(_objUpdater->accessObject(_objUpdater->accessObjects().size() - 1));
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
    //const float test_maxShipSpawnCd = 0.08f;
    //float test_shipSpawnCd = 0.0f;
    while(_run)
    {
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();

        // Test updating some random actions for some units..
        for (world::objects::ObjectInstanceData* obj : _objUpdater->accessObjects()/*_testUnits*/)
        {
            if (obj->getObjType() == 2)
            {
                if (obj->getActionQueue().size() < 2)
                {
                    int r = (std::rand() % 8) + 1;
                    obj->addAction(r);
                }
            }
        }
        // Test spawning players' ships and deploying initial units/objects
        //if (test_shipSpawnCd <= 0.0f)
        //{
        //    const int descentAction = 10;
        //    const int deployAction = 11;
        //    int randX = std::rand() % _worldWidth;
        //    int randZ = std::rand() % _worldWidth;

        //    bool spawned = _objUpdater->spawnObject(randX, randZ, 3, _factions["Neutral"]);
        //    if (spawned)
        //    {
        //        world::objects::ObjectInstanceData* shipObj = _objUpdater->accessObject(_objUpdater->accessObjects().size() - 1);
        //        shipObj->addAction(descentAction);
        //        shipObj->addAction(deployAction);
        //        test_shipSpawnCd = test_maxShipSpawnCd;
        //    }
        //}
        //else
        //{
        //    test_shipSpawnCd -= 1.0f * _deltaTime;
        //}


        _objUpdater->update();

        std::chrono::time_point<std::chrono::high_resolution_clock> endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> delta = endTime - startTime ;
        _deltaTime = delta.count();
        //Debug::log("Delta: " + std::to_string(_deltaTime));
    }
}

void Game::resetChangedFactionsStatus()
{
    std::lock_guard<std::mutex> lock(_mutex_worldState);
    for (const std::pair<std::string, Faction*> faction : _factions)
    {
        if (faction.second->isUpdated())
            faction.second->markUpdated(false);
    }
}

Message Game::addFaction(Server& server, const Client& client, PK_byte* nameData, size_t nameSize)
{
    std::string factionNameStr(nameData, nameSize);
    Debug::log("Attempting to create faction: " + factionNameStr);

    const size_t maxErrMessageSize = 50;
    std::string errorMessage = "";
    Message response(NULL_CLIENT, MESSAGE_TYPE__CreateFaction, 1 + Faction::get_netw_size() + maxErrMessageSize);
    PK_byte success = 0;
    Faction* faction = nullptr;

    std::lock_guard<std::mutex> lock(_mutex_faction);
    auto iter = _factions.find(factionNameStr);
    if(iter == _factions.end())
    {
        const User owner = server.getUser(client);
        if (owner == NULL_USER)
        {
            Debug::log(
                "Client with no logged in user attempted to create a faction",
                Debug::MessageType::ERROR
            );
            return NULL_MESSAGE;
        }
        else if (owner.getFactionName().size() > 0)
        {
            errorMessage = "User already has a faction";
        }
        else
        {
            faction = new Faction(nameData, nameSize);
            _factions.insert(std::make_pair(factionNameStr, faction));
            Debug::log("New faction created successfully. Current faction count: " + std::to_string(_factions.size()));
            server.updateUserFaction(owner, *faction);
            success = 1;
        }
    }
    else
    {
        errorMessage = "Faction with this name already exists";
    }
    response.add((PK_byte*)&success, 1);
    if (faction)
    {
        response.add((PK_byte*)&(*faction), Faction::get_netw_size());
    }
    else
    {
        Faction nullFaction = NULL_FACTION;
        response.add((PK_byte*)&nullFaction, Faction::get_netw_size());
    }
    response.add((PK_byte*)errorMessage.data(), errorMessage.size());
    return response;
}

Message Game::getWorldState(int xPos, int zPos, int observeRadius) const
{
    const int observeRectWidth = (observeRadius * 2) + 1;
    size_t bufSize = sizeof(int32_t) + (observeRectWidth * observeRectWidth) * sizeof(uint64_t);
    Message response(NULL_CLIENT, MESSAGE_TYPE__GetWorldState, bufSize);

    for(int z = zPos - observeRadius; z <= zPos + observeRadius; ++z)
    {
        for(int x = xPos - observeRadius; x <= xPos + observeRadius; ++x)
        {
            // Make sure coords are valid tile coords
            if(x >= 0 && x < _worldWidth && z >= 0 && z < _worldWidth)
            {
                // Should never go out of range? since prev if?.
                int tileIndex = x + z * _worldWidth;
                // .. need to lock so nothing funny happens...
                std::lock_guard<std::mutex> lock(_mutex_worldState);
                response.add((PK_byte*)(_pWorld + tileIndex), sizeof(uint64_t));
            }
            else
            {
                response.incrWritePos(sizeof(uint64_t));
            }
        }
    }
    return response;
}

// Returns all factions' data
Message Game::getAllFactions() const
{
    Message response(NULL_CLIENT, MESSAGE_TYPE__GetAllFactions, _factions.size() * Faction::get_netw_size());

    std::lock_guard<std::mutex> lock(_mutex_worldState);
    for (const std::pair<std::string, Faction*> faction : _factions)
    {
        Faction* factionObj = faction.second;
        response.add((PK_byte*)factionObj->getNetwData(), Faction::get_netw_size());
    }
    return response;
}

Message Game::getChangedFactions() const
{
    std::lock_guard<std::mutex> lock(_mutex_worldState);
    std::vector<Faction*> changedFactions;
    for (const std::pair<std::string, Faction*> faction : _factions)
    {
        if (faction.second->isUpdated())
            changedFactions.push_back(faction.second);
    }
    if (changedFactions.empty())
        return NULL_MESSAGE;
    size_t bufSize = changedFactions.size() * Faction::get_netw_size();
    Message response(NULL_CLIENT, MESSAGE_TYPE__GetChangedFactions, bufSize);

    for (Faction* faction : changedFactions)
    {
        response.add((PK_byte*)&(*faction), Faction::get_netw_size());
    }
    return response;
}

Message Game::getObjInfoLibMsg() const
{
    if (!_objInfoInitialized)
    {
        Debug::log("Attempted to access object info library before it was initialized!");
        return NULL_MESSAGE;
    }
    size_t bufSize = sizeof(int32_t) + _totalObjInfoSize;
    size_t objSize = world::objects::get_netw_objinfo_size();
    PK_byte* buffer = new PK_byte[bufSize];
    memset(buffer, 0, bufSize);

    // Set first 4 bytes to contain message type name
    const int32_t messageType = MESSAGE_TYPE__GetObjInfoLib;
    memcpy(buffer, (const void*)&messageType, sizeof(int32_t));

    size_t bufPos = sizeof(int32_t);

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
                        OBJECT_DATA_STRLEN_NAME +
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
    Message response(NULL_CLIENT, buffer, bufSize);
    delete[] buffer;
    return response;
}

const std::vector<world::objects::ObjectInfo>& Game::getObjInfoLib()
{
    // NOTE: Not sure should we lock here, since the "objTypeLib" is constant
    return _objectInfo;
}

const world::objects::ObjectInfo& Game::getObjInfo(int index) const
{
    if (index >= 0 && index < (int)_objectInfo.size())
    {
        return _objectInfo[index];
    }
    else
    {
        Debug::log("@Game::getObjInfo(int) Accessed invalid obj info lib index");
        return _objectInfo[0]; // index 0 should ALWAYS be "empty object"
    }
}

const Faction* Game::getFaction(const std::string& name) const
{
    std::unordered_map<std::string, Faction*>::const_iterator it = _factions.find(name);
    if (it != _factions.end())
        return (*it).second;
    else
        return nullptr;
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

