#pragma once

#include <cstring>

#define PK_byte char
#define PK_ubyte unsigned char

#define DIR_N 	0
#define DIR_NE 	1
#define DIR_E 	2
#define DIR_SE 	3
#define DIR_S 	4
#define DIR_SW 	5
#define DIR_W 	6
#define DIR_NW 	7

#define USER_ID_LEN 32

#define CLIENT_NAME_LEN 	32
#define USER_NAME_LEN 		32
#define USER_PASSWD_LEN 	32
#define NULL_CLIENT ClientData(0, "null", 4)

struct ClientData
{
	int connSD;
	char name[CLIENT_NAME_LEN];

	int xPos = 0;
	int zPos = 0;
	int observeRadius = 15;

	bool loggedIn = false;

	ClientData(int connectionSD, const char* clientName, size_t nameLen):
		connSD(connectionSD)
	{
		if (nameLen > CLIENT_NAME_LEN)
			nameLen = CLIENT_NAME_LEN;

		memset(name, 0, sizeof(char) * CLIENT_NAME_LEN);
		memcpy(name, clientName, sizeof(char) * nameLen);
	}

	ClientData(const ClientData& other):
		connSD(other.connSD),
		xPos(other.xPos), zPos(other.zPos), observeRadius(other.observeRadius)
	{
		memcpy(name, other.name, sizeof(char) * CLIENT_NAME_LEN);
	}

	bool operator==(const ClientData& other) const
	{
		return (connSD == other.connSD) && (strcmp(name, other.name) == 0);
	}

	bool operator!=(const ClientData& other) const
	{
		return !(other == *this);
	}
};
