#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include "Common.h"
#include "game/Game.h"
#include "RequestHandler.h"
#include <unordered_map>

#include <thread>

class Server
{
private:
	int _serverSD;
	int _port;
	sockaddr_in _address;

	size_t _maxClientCount = 1024;

	const size_t _maxRecvBufLen = 512;
	PK_byte* _pRecvBuf = nullptr;

	Game _game;
	RequestHandler _reqHandler;
	std::thread* _reqHandlerThread = nullptr;

public:

	Server(int port, size_t maxClientCount);
	~Server();

	void beginReqHandler();

	void run();
	

private:
	
	size_t findContentLength(const PK_byte* data, size_t dataLen) const;

	Request convertToReq(PK_byte* data, size_t dataLen, size_t bodyLen, int clientSD);
};
