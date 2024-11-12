#pragma once

#include <mutex>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "../../Proj8315Common/src/Faction.h"
#include "../../Proj8315Common/src/Common.h"
#include "../../Proj8315Common/src/Object.h"
#include "../../Proj8315Common/src/messages/Message.h"
#include "../../Proj8315Common/src/messages/ObjMessages.h"
#include "Client.h"
#include "game/objects/ObjectManager.h"

//#define GAME_WORLD_WIDTH 2000


class Server;


class Game
{
private:
    friend class world::objects::ObjectManager;

    mutable std::mutex _mutex_faction;
    mutable std::mutex _mutex_worldState;

    std::unordered_map<std::string, gamecommon::Faction> _factions;
    std::vector<gamecommon::ObjectInfo> _objectInfo;
    std::unordered_map<std::string, std::vector<float>> _statFloatMapping =
    {
        {"speed", {0.0f, 0.5f, 1.0f}}
    };

    // size of the whole objInfoLib in bytes
    size_t _totalObjInfoSize = 0;
    bool _objInfoInitialized = false;

    int _worldWidth = 32;
    float _updateInterval = 0.25f;
    float _deltaTime = 0.0f;

    uint64_t* _pWorld = nullptr;

    static Game* s_pInstance;

    bool _run = true;

    world::objects::ObjectManager* _pObjManager = nullptr;

    std::vector<world::objects::ObjectInstanceData*> _testUnits;

public:
    Game(int worldWidth);
    Game(const Game&) = delete;
    ~Game();

    void run();
    void resetChangedFactionsStatus();

    gamecommon::Message addFaction(Server& server, const Client& client, const std::string factionName);
    // Returns current "dynamic" world state
    gamecommon::Message getWorldState(int32_t xPos, int32_t zPos, int observeRadius) const;
    // Returns all factions' data
    gamecommon::Message getAllFactions() const;
    gamecommon::Message getChangedFactions() const;
    const std::vector<gamecommon::ObjectInfo>& getObjInfoLib();
    const gamecommon::ObjectInfo& getObjInfo(int index) const;
    const gamecommon::ObjectInfo& getObjInfo(const std::string& name) const;
    const gamecommon::Faction getFaction(const std::string& name) const;
    gamecommon::Faction* accessFaction(const std::string& name);

    uint64_t getTileState(int xPos, int zPos) const;
    uint64_t getTileState(int index) const;
    void setTileState(int xPos, int zPos, uint64_t newState);
    void setTileState(int index, uint64_t newState);

    float getStatValue(const char* name, int statIndex) const;
    float getDeltaTime();
    static Game* get();

    inline const std::vector<gamecommon::ObjectInfo>& getObjLib() const { return _objectInfo; }
    inline std::unordered_map<std::string, std::vector<float>>& getStatFloatMapping() { return _statFloatMapping; }
    inline bool validCoords(int x, int z) const
    {
        int index = x + z * _worldWidth;
        return index >= 0 && index < _worldWidth * _worldWidth;
    }
};
