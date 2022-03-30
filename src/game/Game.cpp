
#include "Game.h"
#include "RequestHandler.h"
#include <cstring>
#include <mutex>

Game::Game()
{
}

Game::~Game()
{
}
/*
Response Game::addFaction(const Request& req)
{
	std::lock_guard<std::mutex> lock(_mutex);

	auto iter = _factions.find(id);
	if(iter == _factions.end())
	{
		_factions.insert(std::make_pair(id, faction));
		return true;
	}
	else
	{
		return false;
	}
}

void Game::parseReqCredentials()
*/
