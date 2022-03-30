#pragma once

#include "game/Game.h"
#include "ByteBuffer.h"
#include <string>
#include <vector>
#include <queue>
#include <mutex>

#define NULL_REQUEST (Request(0, Request::ReqType::REQ_EMPTY, {}))

class Request
{
public:

	enum ReqType
	{
		REQ_EMPTY = 0x0,
		REQ_CREATE_FACTION
	};

private:
	

	int _clientSD;
	ReqType _type;

	std::vector<ByteBuffer> _data;

public:

	Request(int clientSD, ReqType type, const std::vector<ByteBuffer>& data) : 
		_clientSD(clientSD), _type(type), _data(data)
	{}

	Request(const Request& other) :
		_clientSD(other._clientSD), _type(other._type), _data(other._data)
	{}

	~Request() {}

	inline ReqType getType() const { return _type; }
	inline int getClientSD() const { return _clientSD; }
	inline const std::vector<ByteBuffer>& getData() const { return _data; }
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


