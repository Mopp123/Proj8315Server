#pragma once

#include "game/Game.h"
#include "Commands.h"
#include "ByteBuffer.h"
#include <string>
#include <vector>
#include <queue>
#include <mutex>

#define NULL_REQUEST (Request(0, nullptr, 0))

#define USER_ID_LEN 32

class Request
{
private:
	int _clientSD;
	Command _cmd;

public:

	Request(int clientSD, char* data, size_t dataLen) : 
		_clientSD(clientSD), _cmd(data, dataLen)
	{}

	Request(const Request& other) :
		_clientSD(other._clientSD), _cmd(other._cmd)
	{
	}

	~Request() {}

	inline int getClientSD() const { return _clientSD; }
	inline const Command& getCommand() const { return _cmd; }
};


// *"Thread safe" request queue
class RequestQueue
{
private:
	
	mutable std::mutex _mutex;
	std::queue<Request> _requests;

public:

	void push(const Request& req);
	Request pop();
	bool isEmpty() const;
};



class Server;

class RequestHandler
{
private:
	
	Server& _serverRef;
	Game& _gameRef;
	CMDHandler _cmdHandler;

	RequestQueue _reqQueue;
	bool _run = true;

	const std::string _defaultResponseHeader = 
				"HTTP/1.1 200 OK\n"
                                "Content-Type: application/octet-stream\n"
                                "Content-Length: 2\n"
                                "Access-Control-Allow-Origin: *\n"
                                "\n";

public:

	RequestHandler(Server& server, Game& game);

	void addToReqQueue(const Request& req);
	void run();
};


