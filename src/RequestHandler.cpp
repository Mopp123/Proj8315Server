
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
		if(!_reqQueue.isEmpty())
		{
			Request req = _reqQueue.pop();
			Response response = _cmdHandler.processCommand(req.getCommand());
			send(req.getClientSD(), response.getData(), response.getSize(), 0);
			close(req.getClientSD());
		}	
/*
		Request::ReqType reqType = reqToHandle.getType();
		if(reqType != Request::ReqType::REQ_EMPTY)
		{
			std::string userID(reqToHandle.getUserID(), 32);
			Debug::log("Attempting to handle req by user: " + userID);

			int sd = reqToHandle.getClientSD();

			switch(reqType)
			{
				case Request::ReqType::REQ_CREATE_FACTION : 
					break;

				default:
					break;
			}
			
			close(sd);
		}*/
	}
}
