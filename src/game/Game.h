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
public:

	Game();
	~Game();


private:

};
