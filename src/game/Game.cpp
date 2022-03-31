
#include "Game.h"
#include "RequestHandler.h"
#include <cstring>
#include <mutex>

#include "Debug.h"

Game* Game::s_pInstance = nullptr;

Game::Game()
{
	s_pInstance = this;
}

Game::~Game()
{
}

Response Game::addFaction(const std::string& userID, const std::string& factionName)
{
	std::lock_guard<std::mutex> lock(_mutex);
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

Game* Game::get()
{
	return s_pInstance;
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
