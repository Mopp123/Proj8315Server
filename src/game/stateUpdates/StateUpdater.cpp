
#include "StateUpdater.h"
#include "game/Game.h"
#include "Debug.h"

namespace world
{

	StateUpdater::StateUpdater(Game& gameRef, float updateCooldown) : 
		_gameRef(gameRef), _updateCooldown(updateCooldown)
	{
		_timePoint_lastUpdate = std::chrono::high_resolution_clock::now();
	}

	StateUpdater::~StateUpdater()
	{
	}

  
	// Triggers state updater's updateFunc after "cooldown period"
	void StateUpdater::update()
	{
		std::chrono::duration<float> delta =  std::chrono::high_resolution_clock::now() - _timePoint_lastUpdate;
		if(delta.count() >= _updateCooldown)
		{
		        updateFunc();
		        _timePoint_lastUpdate = std::chrono::high_resolution_clock::now();
		}
	}

}
