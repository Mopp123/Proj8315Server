
#include "RequestHandler.h"
#include "Server.h"
#include <mutex>
#include <unistd.h>
#include "Responses.h"

#include "Debug.h"

void RequestQueue::push(const Request& req)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_requests.push(req);
}


Request RequestQueue::pop()
{
	std::lock_guard<std::mutex> lock(_mutex);
	if(!_requests.empty())
	{
		Request front = _requests.front();
		_requests.pop();
		return front;
	}
	else
	{
		return NULL_REQUEST;
	}
}

bool RequestQueue::isEmpty() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	return _requests.empty();
}




// ---------------------------------------------------------

RequestHandler::RequestHandler(Server& server, Game& game) :
	_serverRef(server), _gameRef(game)
{

}


void RequestHandler::addToReqQueue(const Request& req)
{
	_reqQueue.push(req);
	//Debug::log("New req added to queue(type: " + std::to_string(req.getType()) + ")");
}

void RequestHandler::run()
{
	while(_run)
	{
		Request reqToHandle = _reqQueue.pop();
		Request::ReqType reqType = reqToHandle.getType();
		if(reqType != Request::ReqType::REQ_EMPTY)
		{
			int sd = reqToHandle.getClientSD();

			// JUST TESTING ATM...
			if((int)reqType == 1)
			{
				// Get messages...
				// Return only the latest message
				std::string message = _gameRef.getLatestMessage();
				Response response(message.data(), message.size());
				_serverRef.respond(sd, response.getData(), response.getSize());
				
				
			}
			else if((int)reqType == 2)
			{
				// Post message...
				const ByteBuffer data = reqToHandle.getData()[0];
				std::string message = data.getString();
				Debug::log("MESSAGE WAS: " + message);

				_gameRef.addMessage(message);

				Response response(nullptr, 0);
				_serverRef.respond(sd, response.getData(), response.getSize());
			}
		}
	}
}
