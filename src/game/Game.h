#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

#include "Responses.h"
#include "Faction.h"


class Request;

class Game
{
private:

	mutable std::mutex _mutex;

	std::unordered_map<std::string, Faction> _factions;

	static Game* s_pInstance;

public:

	Game();
	Game(const Game&) = delete;
	~Game();

	Response addFaction(const std::string& userID, const std::string& factionName);
	
	static Game* get();

private:

};
