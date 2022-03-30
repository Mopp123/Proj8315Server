
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
	
	sockaddr_in clientAddress;	
	memset(&clientAddress, 0, sizeof(clientAddress));
	socklen_t clientLen = sizeof(clientAddress);
	
	int connSD = accept(_serverSD, (struct sockaddr*)&clientAddress, &clientLen);
	
	if(connSD > 0)
	{
		// Get details of conn..
		getpeername(connSD, (struct sockaddr*)&clientAddress, &clientLen);
		//char* clientAddrName = inet_ntoa(clientAddress.sin_addr);
		//unsigned short clientPort = ntohs(clientAddress.sin_port);
		
		size_t readBytes = read(connSD, _pRecvBuf, _maxRecvBufLen);
		if(readBytes > 0)
		{
			// JUST TESTING ATM!!!
			
			size_t bodySize = findContentLength(_pRecvBuf, readBytes);
			Request req = convertToReq(_pRecvBuf, readBytes, bodySize, connSD);

			//Debug::log("Parsed body size was: " + std::to_string(bodySize));
			_reqHandler.addToReqQueue(req);

			// resert recv buf for next upcomming reqs
			memset(_pRecvBuf, 0, _maxRecvBufLen);
		}
		else
		{
			Debug::log("Error on reading");
			close(connSD);
		}
	}
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


