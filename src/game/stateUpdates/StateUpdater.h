#pragma once

#include <chrono>

class Game;

namespace world
{

	class StateUpdater
	{
	protected:
		Game& _gameRef;

		std::chrono::time_point<std::chrono::high_resolution_clock> _timePoint_lastUpdate;
		float _updateCooldown = 1.0f;
	
	public:
		
		StateUpdater(Game& gameRef, float updateCooldown);
		virtual ~StateUpdater();

		// Triggers state updater's updateFunc after "cooldown period"
		void update();

	protected:
		// Each different kind of state updater does the actual updating logic here
		virtual void updateFunc() = 0;

	};
}
