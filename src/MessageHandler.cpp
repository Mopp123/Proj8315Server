
#include <mutex>
#include <unistd.h>
#include <vector>

#include "MessageHandler.h"
#include "Server.h"
#include "msgs/General.h"
#include "msgs/WorldState.h"
#include "Debug.h"


Message::Message(const ClientData& client, char* pData, size_t dataLen) : 
	_client(client)
{
	if (pData && dataLen > MESSAGE_MIN_DATA_SIZE && dataLen <= MESSAGE_MAX_DATA_SIZE)
	{
		_dataLen = dataLen;
		_pData = new PK_byte[_dataLen];
		memcpy(_pData, pData, _dataLen);
	}
	else
	{
		std::string dataStr(pData, dataLen);
		const std::string msg = "Attempted to create Message instance with invalid data. raw data: " + dataStr + " data size: " + std::to_string(dataLen);
	}
}

Message::Message(const Message& other) :
	_client(other._client)
{
	if (other._pData && other._dataLen > 0)
	{
		_dataLen = other._dataLen;
		_pData = new PK_byte[_dataLen];
		memcpy(_pData, other._pData, _dataLen);
	}
}

Message::~Message() 
{
	delete[] _pData;
}

int32_t Message::getType() const
{
	int32_t type = -1;
	if(_pData && _dataLen >= MESSAGE_MIN_DATA_SIZE)
		memcpy(&type, _pData, MESSAGE_ENTRY_SIZE__header);

	return type;
}

// ---------------------------------------------------------

void MessageQueue::push(const Message& msg)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_messages.push(msg);
}


Message MessageQueue::pop()
{
	std::lock_guard<std::mutex> lock(_mutex);
	if(!_messages.empty())
	{
		Message front = _messages.front();
		_messages.pop();
		return front;
	}
	else
	{
		return NULL_MESSAGE;
	}

}

bool MessageQueue::isEmpty() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _messages.empty();
}

// ---------------------------------------------------------

MessageHandler::MessageHandler(Server& server, Game& game) :
	_serverRef(server), _gameRef(game)
{
	_pRecvBuf = new PK_byte[_maxRecvBufLen];
	memset(_pRecvBuf, 0, _maxRecvBufLen);

	_msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__CreateFaction, msgs::msg_createNewFaction));
	_msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__FetchServerMessage, msgs::msg_fetchServerMessage));
	_msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__FetchWorldState, msgs::msg_fetchWorldState));
	_msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__ServerShutdown, msgs::msg_serverShutdown));
}

MessageHandler::~MessageHandler()
{
	delete[] _pRecvBuf;
}

void MessageHandler::addToMsgQueue(const Message& msg)
{
	_msgQueue.push(msg);
}

void MessageHandler::run()
{
	while(_run)
	{
		// Check for new messages from each client connection
		std::vector<ClientData> currentClients = _serverRef.getClientConnections();
		for (ClientData& client : currentClients)
		{
			size_t readBytes = read(client.connSD, _pRecvBuf, _maxRecvBufLen);
			
			std::string fullMessage(_pRecvBuf, readBytes);
			Debug::log("\nFULL MESSAGE:\n" + fullMessage + "\n");
			
			if (readBytes > 0)
			{
				Message msg(client, _pRecvBuf, readBytes);
				Message response = processMessage(msg);
				if (response != NULL_MESSAGE)
					send(msg.getClient().connSD, response.getData(), response.getSize(), 0);
				
				// JUST FOR TESTING..
				/*
				std::string message = "Welcome websockets yey!";
				Debug::log("Attempting to send...");
				send(client.connSD, message.data(), message.size(), 0);
				*/
			}
			memset(_pRecvBuf, 0, _maxRecvBufLen);
		}
	}
}

Message MessageHandler::processMessage(const Message& msg)
{
	const int32_t msgType = msg.getType();
	auto iter = _msgFuncMapping.find(msgType);
	if(iter != _msgFuncMapping.end())
	{
		return (*_msgFuncMapping[msgType])(msg);
	}
	else
	{
		Debug::log("Failed to process command. Couldn't find cmd name");
		return NULL_MESSAGE;
	}
}
