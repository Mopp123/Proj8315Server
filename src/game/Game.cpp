
#include "Game.h"
#include <cstring>

Game::Game()
{
	// prealloc..
	for(int i = 0; i < _maxMsgCount; ++i)
	{
		_messages.push_back("");
	}

	addMessage("Test message");
}

Game::~Game()
{
}


void Game::addMessage(std::string msg)
{
	std::lock_guard<std::mutex> lock(_mutex);

	if(_lastMsgIndex + 1 < _maxMsgCount)
	{
		_lastMsgIndex++;
	}
	else
	{
		for(int i = 1; i < _maxMsgCount; ++i)
		{
			_messages[i - 1] = _messages[i];
		}
	}

	_messages[_lastMsgIndex] = msg;
}

std::string Game::getLatestMessage() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _messages[_lastMsgIndex];
}

std::vector<std::string> Game::getMessageBoardContent() const
{
	std::lock_guard<std::mutex> lock(_mutex);

	return _messages;
}
