
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <string_view>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <vector>
#include <iostream>

#include "Server.h"

#include "game/world/Tile.h"
#include "Debug.h"
#include <iostream>
#include <string>

// Outputs either 0 or 1 to "outBit", depending on is bit set in "val" at "pos"
template<typename T>
void get_bit(const T& val, int pos, PK_byte& outBit)
{
	outBit = (val >> pos) & 0x1;
}

// Sets bit at "target" in "target" to bit
void set_bit(uint64_t& target, int pos, PK_byte bit)
{
	int mask = 1 << pos;
	target = ((target & ~mask) | bit << pos);
}

// Sets area in "target" to match bits in "toSet" starting from "startPos"
// *Currently bit count has to match that inputted "toSet" bitcount..
void set_area(uint64_t& target, uint32_t toSet, int startPos, int bitCount)
{
	for(int j = 0; j < bitCount; ++j)
	{
		uint64_t i = j + startPos;
		
		PK_byte bit = 0;
		get_bit<uint32_t>(toSet, j, bit);
		
		uint64_t bit64 = (uint64_t)bit;
		uint64_t mask = (uint64_t)1 << i;
		target = ((target & ~mask) | bit64 << i);
	}
}



void convert_area(const uint64_t& inValue, uint32_t& outValue, int pos, int count)
{
	for(int i = 0; i < count; ++i)
	{
		int j = pos + i;
		//int mask = 1 << i;

		// Returns, is bit at pos "i" set in value "val"
		PK_byte bit = (inValue >> j) & 0x1;
		
		
		outValue = ((outValue & ~(1 << i)) | bit << i);

		//set_bit(&outValue, i, bit);



	}
}


void display_bits(uint64_t val)
{
	for(int i = 63; i >= 0; --i)
	{
		PK_byte bit = 0;
		get_bit<uint64_t>(val, i, bit);
		std::cout << std::to_string(bit);
	}
	std::cout << "\n\n";
}


void display_bits(uint32_t val)
{
	for(int i = 31; i >= 0; --i)
	{
		PK_byte bit = 0;
		get_bit<uint32_t>(val, i, bit);
		std::cout << std::to_string(bit);
	}
	std::cout << "\n\n";
}

int main(const int argc, const char** argv)
{
	
	uint64_t val = 0;
	/*
	set_bit(&val, 0, 1);
	set_bit(&val, 1, 0);
	set_bit(&val, 2, 1);
	set_bit(&val, 3, 1);
	set_bit(&val, 4, 0);
	*/
	uint32_t toSet = 420;
	set_area(val, toSet, 0, 32);

	uint32_t toSet2 = 123;
	set_area(val, toSet2, 32, 32);
	
	
	uint32_t test = 0;
	convert_area(val, test, 0, 32);

	Debug::log("full: ");
	display_bits(val);

	Debug::log("test: ");
	display_bits(test);

	Debug::log("Full value: " + std::to_string(val) + " 	Test was: " + std::to_string(test));


	/*

	Server server(51421, 1024);

	server.beginReqHandler();
	
	while(true)
	{
		server.run();
	}
*/
	return 0;
}


/*
int main(int argc, char *argv[])
{
     	int socketFd;
	int port = 51421;	
	
	const int maxClientCount = 1024;
	int clientfds[maxClientCount];
	memset(clientfds, 0, sizeof(int) * maxClientCount);

	fd_set readfds;

	
     	socklen_t clientLen;

	size_t maxRecvBufSize = 512;
     	char receiveBuffer[maxRecvBufSize];
 
	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;

     	socketFd = socket(AF_INET, SOCK_STREAM, 0);
     	if (socketFd < 0) 
		std::cout << "Error on socket creation\n";

	bzero((char *) &serverAddress, sizeof(serverAddress));
     	serverAddress.sin_family = AF_INET;
     	serverAddress.sin_addr.s_addr = INADDR_ANY;
     	serverAddress.sin_port = htons(port);
     	if (bind(socketFd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) 	
		std::cout << "Failed to bind address to socket\n";

	
	std::cout << "Started listening for messages...\n";

	listen(socketFd,5);
     	clientLen = sizeof(clientAddress);
     	
	while(true)
	{
		
		// First clear existing fd_set
		FD_ZERO(&readfds);
		
		// Add the "server's socket" to set
		FD_SET(socketFd, &readfds);
		// We need the highest file descriptor for the "select"-func?
		int maxsd = socketFd;
		
		for(int i = 0; i < maxClientCount; ++i)
		{
			int sd = clientfds[i];
			if(sd > 0)
				FD_SET(sd, &readfds);
			
			if(sd > maxsd)
				maxsd = sd;
		}
		
		// Wait for activity on one of the sockets..
		int activity = select(maxsd + 1, &readfds, NULL, NULL, NULL);
		
		if(activity < 0)
		{
			std::cout << "Error on select\n";
		}

		int newSocket;
		
		// Check if something happened on our master socket
		// -> means we got a new connection...
		if(FD_ISSET(socketFd, &readfds))
		{
			newSocket = accept(socketFd, (struct sockaddr*)&clientAddress, &clientLen);
			if(newSocket < 0)
			{
				std::cout << "Error on acccept\n";
			}
			else
			{
				// If conn successfully "accepted" -> send welcome message...
				
				std::string responseData = 
				"HTTP/1.1 200 OK\n"
				"Content-Type: application/octet-stream\n"
				"Content-Length: 2\n"
				"Access-Control-Allow-Origin: *\n"
				"\n"
				"hi\n";
			
				int sentBytes = send(newSocket, responseData.data(), responseData.size(), 0);
				if(sentBytes != (int)responseData.size())
				{
					std::cout << "Error on welcome send\n";
				}
				// Add this new connection to our connections list
				for(int i = 0; i < maxClientCount; ++i)
				{
					if(clientfds[i] == 0)
					{
						clientfds[i] = newSocket;
						break;
					}
				}
			}
		}

		
		// Check activity on other "connected sockets"
		for(int i = 0; i < maxClientCount; ++i)
		{
			int sd = clientfds[i];
			if(FD_ISSET(sd, &readfds))
			{
				// Check if this was for closing conn...
				int readVal = read(sd, receiveBuffer, maxRecvBufSize);
				if(readVal == 0)
				{
					//close(sd);
					//clientfds[i] = 0;
					//std::cout << "Client disconnected\n";
				}
				// If some other activity...
				else
				{
					std::cout << "Received:\n" << receiveBuffer << std::endl;
				}
			}
		}
		
	
		bzero(receiveBuffer,maxRecvBufSize);
	}
	
	close(socketFd);
	
	return 0; 
}
*/


