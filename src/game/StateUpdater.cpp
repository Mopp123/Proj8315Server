
#include "StateUpdater.h"
#include "game/Game.h"
#include "Debug.h"

namespace world
{

	StateUpdater::StateUpdater(Game& gameRef) : 
		_gameRef(gameRef)
	{
	}

	StateUpdater::~StateUpdater()
	{
	}
  
	// Triggers state updater's updateFunc after "cooldown period"
	void StateUpdater::update()
	{
		updateFunc();
	}
}
