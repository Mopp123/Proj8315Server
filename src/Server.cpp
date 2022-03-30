
#include "Server.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <string>

#include "Debug.h"


Server::Server(int port, size_t maxClientCount) : 
	_port(port), _maxClientCount(maxClientCount), _reqHandler(*this, _game)
{

	// Create socket
	_serverSD = socket(AF_INET, SOCK_STREAM, 0);
	if(_serverSD < 0)
	{
		Debug::log("Failed to create socket");
	}
	
	// Config address
	memset(&_address, 0, sizeof(_address));
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);

	// Bind socket to the address..
	if(bind(_serverSD, (struct sockaddr *)&_address, sizeof(_address)) < 0)
	{
		Debug::log("Failed to bind socket to address");
	}

	listen(_serverSD, 5);
	Debug::log("Started server on port:" + std::to_string(port));

	// prealloc our recv buf
	_pRecvBuf = new PK_byte[_maxRecvBufLen];
	memset(_pRecvBuf, 0, _maxRecvBufLen);
}

Server::~Server()
{
	delete[] _pRecvBuf;

	if(_reqHandlerThread)
	{
		_reqHandlerThread->join();
		delete _reqHandlerThread;
	}

	close(_serverSD);
}



void Server::beginReqHandler()
{
	if(!_reqHandlerThread)
		_reqHandlerThread = new std::thread(&RequestHandler::run, &_reqHandler);
	else
		Debug::log("Attempted to launch RequestHandler multiple times!");
}


void Server::run()
{
	// Clear existing state of our fd_set
	FD_ZERO(&_readfds);
	FD_SET(_serverSD, &_readfds);
	_maxSD = _serverSD;

	// Update fd_set to match our "connected clients"
	for(auto client : _clientSDs)
	{
		int clientSD = client.second;
		if(clientSD > 0)
		{
			FD_SET(clientSD, &_readfds);
			_maxSD = clientSD > _maxSD ? clientSD : _maxSD;
		}
	}

	// Wait for activity in one of our sockets..
	int activity = select(_maxSD + 1, &_readfds, NULL, NULL, NULL);

	if(activity < 0)
	{
		Debug::log("Error on select-func");
	}
	
	
	sockaddr_in clientAddress;	
	memset(&clientAddress, 0, sizeof(clientAddress));
	socklen_t clientLen = sizeof(clientAddress);
	
	// Check if something happened to our "server socket descriptor"
	// 	-> That indicates we have new client attempting to connect...
	if(FD_ISSET(_serverSD, &_readfds))
	{

		int newSocket = accept(_serverSD, (struct sockaddr*)&clientAddress, &clientLen);
		

		if(newSocket < 0)
		{
			Debug::log("Failed to accept connection");
		}
		else
		{

			// Get details of conn..
			getpeername(newSocket, (struct sockaddr*)&clientAddress, &clientLen);
			char* clientAddrName = inet_ntoa(clientAddress.sin_addr);
			std::string clientAddr_str(clientAddrName);
			auto iter = _clientSDs.find(clientAddr_str);
			if(iter != _clientSDs.end())
			{
				_clientSDs[std::string(clientAddrName)] = newSocket;
			}
			else if(_clientSDs.size() < _maxClientCount)
			{
				Debug::log("new client connected(" + clientAddr_str + ")");
				_clientSDs.insert(std::make_pair(clientAddr_str, newSocket));
			}
			else
			{

				close(newSocket);
			}

			/*
			if(_clientSDs.find(newSocket) == _clientSDs.end())
			{
			std::string responseData =
                                "HTTP/1.1 200 OK\n"
                                "Content-Type: application/octet-stream\n"
                                "Content-Length: 0\n"
                                "Access-Control-Allow-Origin: *\n"
                                "\n";

                                int sentBytes = send(newSocket, responseData.data(), responseData.size(), 0);
                                if(sentBytes != (int)responseData.size())
                                {
                                        std::cout << "Error on welcome send\n";
                                }
                                // Add this new connection to our connections list
				//_clientSDs.push_back(newSocket);
				_clientSDs[newSocket] = 1;
				Debug::log("Connection accepted (sd: " + std::to_string(newSocket) + ")");
			}*/
		}
	}

	// Check activity on "already connected sockets"
	for(auto client : _clientSDs)
	{
		int sd = client.second;

		if(FD_ISSET(sd, &_readfds))
		{
			int readVal = read(sd, _pRecvBuf, _maxRecvBufLen);
			if(readVal > 0)
			{
				
				// JUST TESTING ATM!!!
				
				size_t bodySize = findContentLength(_pRecvBuf, readVal);
				Request req = convertToReq(_pRecvBuf, readVal, bodySize, sd);

				//Debug::log("Parsed body size was: " + std::to_string(bodySize));
				_reqHandler.addToReqQueue(req);
			}
		}
		memset(_pRecvBuf, 0, _maxRecvBufLen);
	}

}


void Server::respond(int clientSD, const PK_byte* data, size_t size)
{
	send(clientSD, (void*)data, size, 0);
}


size_t Server::findContentLength(const PK_byte* data, size_t dataLen) const
{
	std::string msg(data, dataLen);
	const std::string toFind = "Content-Length: ";

	size_t pos = msg.find(toFind);
	if(pos != std::string::npos)
	{
		std::string len_str(data + pos + toFind.size());
		return std::stoi(len_str);
	}
	return 0;
}

Request Server::convertToReq(PK_byte* data, size_t dataLen, size_t bodyLen, int clientSD)
{
	
	PK_byte messageType = data[dataLen - bodyLen];
	std::vector<ByteBuffer> reqBody;
	
	// JUST TESTING... (we assume that body contains messageType(byte) + message(str))
	if(messageType == 2)
	{
		ByteBuffer body(data + (dataLen - bodyLen + 1), bodyLen - 1);
		reqBody.push_back(body);
	}



	Request req(clientSD, (Request::ReqType)messageType, reqBody);
	return req;
}


