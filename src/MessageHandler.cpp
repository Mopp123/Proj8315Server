
#include <mutex>
#include <unistd.h>
#include <vector>

#include "MessageHandler.h"
#include "Server.h"
#include "msgs/General.h"
#include "msgs/WorldState.h"
#include "Debug.h"


Message::Message(const ClientData& client, char* pData, size_t dataLen) : 
	_client(client), _dataLen(dataLen)
{
	if (pData && _dataLen >= MESSAGE_MIN_DATA_SIZE && _dataLen <= MESSAGE_MAX_DATA_SIZE)
	{
		_pData = new PK_byte[_dataLen];
		memcpy(_pData, pData, _dataLen);
	}
	else if (client != NULL_CLIENT)
	{
		std::string dataStr(pData, dataLen);
		const std::string msg = "Attempted to create Message instance with invalid data. raw data: " + dataStr + " data size: " + std::to_string(dataLen) + " Maximum data size is: " + std::to_string(MESSAGE_MAX_DATA_SIZE);
		Debug::log(msg);
	}
}

Message::Message(const Message& other) :
	_client(other._client), _dataLen(other._dataLen)
{
	if (other._pData && other._dataLen >= MESSAGE_MIN_DATA_SIZE)
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

	//Debug::log("Received message of type: " + std::to_string(type));

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

	_msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__GetServerMessage, msgs::get_server_message));
	_msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__UserLogin, msgs::user_login));
	_msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__GetObjInfoLib, msgs::fetch_obj_type_lib));
	_msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__CreateFaction, msgs::create_new_faction));
	_msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__UpdateObserverProperties, msgs::update_observer));
	//_msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__GetWorldState, msgs::msg_fetchWorldState));
	_msgFuncMapping.insert(std::make_pair(MESSAGE_TYPE__ServerShutdown, msgs::server_shutdown));
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
			ssize_t readBytes = read(client.connSD, _pRecvBuf, _maxRecvBufLen);
			
			//std::string fullMessage(_pRecvBuf, readBytes);
			//Debug::log("\nFULL MESSAGE:\n" + fullMessage + "\n");
			
			if (readBytes > 0)
			{
				Message msg(client, _pRecvBuf, readBytes);
				Message response = processMessage(msg);
				if (response != NULL_MESSAGE)
				{
					Debug::log("SENDING WITH SIZE: " + std::to_string(response.getSize()));
					ssize_t sentBytes = send(msg.getClient().connSD, response.accessData(), response.getSize(), MSG_NOSIGNAL);
					if (sentBytes < 0)
						Debug::log("ERROR ON SENDING!");
				}
			}
			memset(_pRecvBuf, 0, _maxRecvBufLen);

			// Broadcast game world state each tick
			Message worldStateMsg = _gameRef.getWorldState(client.xPos, client.zPos, client.observeRadius);
			ssize_t sentBytes = send(client.connSD, worldStateMsg.accessData(), worldStateMsg.getSize(), MSG_NOSIGNAL);
			// TODO: Better probing for dropped connections!
			if (sentBytes <= 0)
				_serverRef.disconnectClient(client.connSD);
		}
	}
}

Message MessageHandler::processMessage(Message& msg)
{
	const int32_t msgType = msg.getType();

	Debug::log("___TEST___received message of type: " + std::to_string(msgType));

	auto iter = _msgFuncMapping.find(msgType);
	if(iter != _msgFuncMapping.end())
	{
		return (*_msgFuncMapping[msgType])(_serverRef, msg);
	}
	else
	{
		Debug::log("Failed to process message. Couldn't find message name");
		return NULL_MESSAGE;
	}
}
