#include "Game.h"
#include "../../Proj8315Common/src/Common.h"
#include "../../Proj8315Common/src/Tile.h"
#include "../../Proj8315Common/src/messages/WorldMessages.h"
#include "Server.h"
#include "MessageHandler.h"

#include "objects/Object.h"
#include "objects/ObjectManager.h"
#include "world/WorldGenerator.h"
#include "DatabaseManager.h"
#include "../Debug.h"

#include <cstring>
#include <mutex>
#include <unordered_set>
#include <chrono>
#include <utility>
#include <string>


using namespace gamecommon;

Game* Game::s_pInstance = nullptr;

Game::Game(int worldWidth) :
    _worldWidth(worldWidth)
{
    s_pInstance = this;

    // TODO: Maybe some kind of "object info library" class
    // so we don't need to manage that shit here

    // initialize object types "library"
    _objectInfo = world::objects::load_obj_info_db();
    // determine total size of the objLib in bytes
    _totalObjInfoSize = _objectInfo.size() * get_netw_objinfo_size();
    _objInfoInitialized = true;


    _pObjManager = new world::objects::ObjectManager(*this);


    // Generate world tiles
    _pWorld = new uint64_t[_worldWidth * _worldWidth];
    memset((void*)_pWorld, 0, sizeof(uint64_t) * _worldWidth * _worldWidth);

    unsigned int worldGenSeed = 4718;
    // This should be max val of terrain elevation(atm max val of 5 bit uint)
    // NOTE: atm thinking of reverting to use just 4 bits (max val = 15)
    int maxElevationVal = 31;
    world::generate_world(
        _pWorld,
        _worldWidth,
        maxElevationVal,
        worldGenSeed,
        128,
        20,
        _pObjManager
    );



    // Load factions from db if exists
    QueryResult factionsResult = DatabaseManager::exec_query(
        "SELECT * FROM factions;"
    );
    Debug::log("___TEST___LOADING EXISTING FACTIONS (" + std::to_string(factionsResult.result.size()) + ")");
    for (int i = 0; i <  factionsResult.result.size(); ++i)
    {
        std::string id = factionsResult.getValue<std::string>(i, DATABASE_COLUMN__FACTIONS__ID);
        std::string name = factionsResult.getValue<std::string>(i, DATABASE_COLUMN__FACTIONS__NAME);
        Faction faction(id, name);
        _factions.insert(std::make_pair(name, faction));
        Debug::log("    Added exiting faction: " + name + "(" + id + ")");
    }


    Faction neutralFaction = _factions["Neutral"];
    // TODO: Asserts...
    if (neutralFaction == NULL_FACTION)
    {
        Debug::log(
            "Starting server requires 'Neutral' faction to exist",
            Debug::MessageType::FATAL_ERROR
        );
    }

    // Testing movement with these objs
    for (int i = 0; i < 1000; ++i)
    {
    	int randX = std::rand() % _worldWidth;
    	int randY = std::rand() % _worldWidth;
    	if (_pObjManager->spawnObject(randX, randY, "Unit Test", neutralFaction))
    	    _testUnits.push_back(_pObjManager->accessObject(_pObjManager->accessObjects().size() - 1));
    }
}

Game::~Game()
{
    delete _pObjManager;
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
        for (world::objects::ObjectInstanceData* obj : _pObjManager->accessObjects()/*_testUnits*/)
        {
            if (obj->getObjType() == 3)
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


        _pObjManager->update();

        std::chrono::time_point<std::chrono::high_resolution_clock> endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> delta = endTime - startTime ;
        _deltaTime = delta.count();
        //Debug::log("Delta: " + std::to_string(_deltaTime));
    }
}

// NOTE: Inefficient?
void Game::resetChangedFactionsStatus()
{
    std::lock_guard<std::mutex> lock(_mutex_worldState);
    // NOTE: interpreting warnings as errors dont allow using std::pair<std::string, Faction>&
    // but accepts auto&. ..weird?
    for (auto& faction : _factions)
    {
        if (faction.second.isUpdated())
            faction.second.markUpdated(false);
    }
}

Message Game::addFaction(Server& server, const Client& client, const std::string factionName)
{
    Debug::log("Attempting to create faction: " + factionName);
    bool success = false;
    Faction faction = NULL_FACTION;

    std::string errorMessage = "";

    std::lock_guard<std::mutex> lock(_mutex_faction);
    auto iter = _factions.find(factionName);
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
            // Attempt inserting into db before adding to actual "game world"
            User user = server.getUser(client);
            if (user != NULL_USER)
            {
                QueryResult insertFactionResult = DatabaseManager::exec_query(
                    "INSERT INTO factions(user_id, name) VALUES('" + user.getID() + "', '" + factionName + "') RETURNING id;"
                );
                if (insertFactionResult.status == QUERY_STATUS__SUCCESS && insertFactionResult.result.size() == 1)
                {
                    Faction newFaction(
                        insertFactionResult.getValue<std::string>(0, DATABASE_COLUMN__FACTIONS__ID),
                        factionName
                    );
                    faction = newFaction;
                    _factions.insert(std::make_pair(factionName, newFaction));
                    Debug::log("New faction created successfully. Current faction count: " + std::to_string(_factions.size()));
                    server.updateUserFaction(owner, faction);
                    success = true;
                }
                else
                {
                    Debug::log(
                        "Failed to add new faction. Database error: " + insertFactionResult.errorMsg,
                        Debug::MessageType::FATAL_ERROR
                    );
                    errorMessage = "Internal server error";
                }
            }
        }
    }
    else
    {
        const std::string errStr = "Faction with this name already exists";
        Debug::log("Failed to create faction: " + factionName + " " + errStr);
        errorMessage = errStr;
    }
    return CreateFactionResponse(success, errorMessage, faction);
}

Message Game::getWorldState(int32_t xPos, int32_t zPos, int observeRadius) const
{
    size_t bufSize = MESSAGE_REQUIRED_SIZE__WorldStateMsg;

    int32_t msgType = MESSAGE_TYPE__WorldState;
    GC_byte* pBuf = new GC_byte[bufSize];
    memset(pBuf, 0, bufSize);
    // add msg typ
    memcpy(pBuf, &msgType, sizeof(int32_t));
    int writePos = sizeof(int32_t);
    // add requested tile coords
    memcpy(pBuf + writePos, &xPos, sizeof(int32_t));
    memcpy(pBuf + (writePos + sizeof(int32_t)), &zPos, sizeof(int32_t));
    writePos += sizeof(int32_t) * 2;

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

                memcpy(pBuf + writePos, (GC_byte*)(_pWorld + tileIndex), sizeof(uint64_t));
            }
            writePos += sizeof(uint64_t);
        }
    }
    Message msg(pBuf, bufSize, MESSAGE_REQUIRED_SIZE__WorldStateMsg);
    delete[] pBuf;
    return msg;
}

// Returns all factions' data
// TODO: Create message of type "GetAllFactions" which takes just takes all factions as parameter
Message Game::getAllFactions() const
{
    std::vector<Faction> factionsList;
    factionsList.reserve(_factions.size());
    std::lock_guard<std::mutex> lock(_mutex_worldState);
    for (const auto& faction : _factions)
        factionsList.emplace_back(faction.second);

    return FactionListResponse(factionsList);
}

// TODO: Create message of type "GetChangedFactions" which takes just takes changed factions as parameter
Message Game::getChangedFactions() const
{
    std::lock_guard<std::mutex> lock(_mutex_worldState);
    std::vector<Faction> changedFactions;
    for (auto& faction : _factions)
    {
        if (faction.second.isUpdated())
            changedFactions.push_back(faction.second);
    }
    if (changedFactions.empty())
        return NULL_MESSAGE;

    return UpdatedFactionsMsg(changedFactions);
    // OLD BELOW!!!
    /*
    std::lock_guard<std::mutex> lock(_mutex_worldState);
    std::vector<Faction> changedFactions;
    for (auto& faction : _factions)
    {
        if (faction.second.isUpdated())
            changedFactions.push_back(faction.second);
    }
    if (changedFactions.empty())
        return NULL_MESSAGE;
    //Message response(NULL_CLIENT, MESSAGE_TYPE__GetChangedFactions, bufSize);
    int32_t msgType = MESSAGE_TYPE__GetChangedFactions;
    size_t bufSize = sizeof(int32_t) + changedFactions.size() * Faction::get_netw_size();
    GC_byte buf[bufSize];
    memset(buf, 0, bufSize);
    memcpy(buf, &msgType, sizeof(int32_t));
    int writePos = sizeof(int32_t);

    for (const Faction& faction : changedFactions)
    {
        //response.add((PK_byte*)&(*faction), Faction::get_netw_size());
        size_t factionSize = Faction::get_netw_size();
        memcpy(buf + writePos, faction.getNetwData(), factionSize);
        writePos += factionSize;
    }
    // NOTE: just a hack atm
    return Message(buf, bufSize, bufSize);
    */
}

const std::vector<ObjectInfo>& Game::getObjInfoLib()
{
    // NOTE: Not sure should we lock here, since the "objTypeLib" is constant
    return _objectInfo;
}

const ObjectInfo& Game::getObjInfo(int index) const
{
    if (index >= 0 && index < (int)_objectInfo.size())
    {
        return _objectInfo[index];
    }
    else
    {
        Debug::log(
            "@Game::getObjInfo(1) Accessed invalid obj info lib index",
            Debug::MessageType::WARNING
        );
        return _objectInfo[0]; // index 0 should ALWAYS be "empty object"
    }
}

const ObjectInfo& Game::getObjInfo(const std::string& name) const
{
    /*
    Debug::log(
        "@Game::getObjInfo(2)"
        "Getting object using name. This is discouraged due to its' slowness. "
        "You should use Game::getObjInfo(1) with index instead!",
        Debug::MessageType::WARNING
    );
    */
    for (const ObjectInfo& objInfo : _objectInfo)
    {
        std::string objName(objInfo.name);
        if (objName == name)
            return objInfo;
    }
    Debug::log(
        "@Game::getObjInfo(2) Failed to find object using name: " + name,
        Debug::MessageType::WARNING
    );
    return _objectInfo[0]; // index 0 should ALWAYS be "empty object"
}

const Faction Game::getFaction(const std::string& name) const
{
    std::unordered_map<std::string, Faction>::const_iterator it = _factions.find(name);
    if (it != _factions.end())
        return (*it).second;
    else
        return NULL_FACTION;
}

Faction* Game::accessFaction(const std::string& name)
{
    std::unordered_map<std::string, Faction>::iterator it = _factions.find(name);
    if (it != _factions.end())
        return &it->second;
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

