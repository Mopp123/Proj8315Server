
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

#include "BitShit.hpp"

#include "game/world/Tile.h"


int main(const int argc, const char** argv)
{
	/*uint64_t tileState = 0;
	uint32_t uid = 147126;

	world::Tile tile{tileState};

	world::set_tile_uid(tile.state, uid);

	Debug::log("uid: " + std::to_string(world::get_tile_uid(tile.state)));
	*/
	

	Server server(51421, 1024);

	server.beginReqHandler();
	
	while(true)
	{
		server.run();
	}

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


