#pragma once

// #include <cstring>
#include <string>
#include <vector>
#include <queue>
#include <mutex>

#include "Common.h"
#include "Debug.h"
#include "game/Game.h"
#include "ByteBuffer.h"

#define USER_ID_LEN 32
#define MESSAGE_MAX_DATA_SIZE (100*100*8)
#define MESSAGE_MIN_DATA_SIZE sizeof(int32_t)

#define MESSAGE_ENTRY_SIZE__header sizeof(int32_t)

#define NULL_MESSAGE Message(NULL_CLIENT, nullptr, 0)

#define MESSAGE_TYPE__GetServerMessage          0x1
#define MESSAGE_TYPE__UserLogin            		0x2
#define MESSAGE_TYPE__UserLogout            	0x3
#define MESSAGE_TYPE__GetObjInfoLib             0x4
#define MESSAGE_TYPE__CreateFaction             0x5
#define MESSAGE_TYPE__GetWorldState             0x6
#define MESSAGE_TYPE__UpdateObserverProperties 	0x7
#define MESSAGE_TYPE__ServerShutdown            0x8


class Message
{
private:
	ClientData _client = NULL_CLIENT; // client which sent this message
	PK_byte* _pData = nullptr;
	size_t _dataLen = 0;
	
public:
	Message(const ClientData& client, char* pData, size_t dataLen);
	Message(const Message& other);
	~Message();

	int32_t getType() const;

	inline const ClientData& getClient() const { return _client; }
	inline PK_byte* accessData() { return _pData; }
	inline size_t getSize() const { return _dataLen; }
	
	bool operator==(const Message& other)
	{
		return (_client == other._client) && 
			(_dataLen == other._dataLen) && 
			(memcmp(_pData, other._pData, _dataLen));
	}

	bool operator!=(const Message& other)
	{
		return !(*this == other);
	}
};


// *Thread safe message queue
class MessageQueue
{
private:
	mutable std::mutex _mutex;
	std::queue<Message> _messages;

public:
	void push(const Message& msg);
	Message pop();
	bool isEmpty() const;
};


class Server;


class MessageHandler
{
private:
	Server& _serverRef;
	Game& _gameRef;
	
	const size_t _maxRecvBufLen = 512;
	PK_byte* _pRecvBuf = nullptr;

	// specifies what happens on each different kind of client message
	std::unordered_map<int32_t, Message(*)(Server& server, Message&)> _msgFuncMapping;

	MessageQueue _msgQueue;
	bool _run = true;

public:
	MessageHandler(Server& server, Game& game);
	~MessageHandler();

	void addToMsgQueue(const Message& msg);
	void run();

private:
	Message processMessage(Message& msg);
};

