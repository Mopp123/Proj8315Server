#pragma once

#include <mutex>
#include <string>
#include <vector>


class Game
{
private:

	mutable std::mutex _mutex;

	const int _maxMsgCount = 16;
	int _lastMsgIndex = 0;
	std::vector<std::string> _messages;

public:

	Game();
	~Game();

	void addMessage(std::string msg);
	std::string getLatestMessage() const;
	std::vector<std::string> getMessageBoardContent() const;

	
};
