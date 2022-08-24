#pragma once

class Game;

namespace world
{

	class StateUpdater
	{
	protected:
		Game& _gameRef;

		float _updateCooldown = 1.0f;
	public:
		
		StateUpdater(Game& gameRef);
		virtual ~StateUpdater();

		// Triggers state updater's updateFunc after "cooldown period"
		void update();

	protected:
		// Each different kind of state updater does the actual updating logic here
		virtual void updateFunc() = 0;

	};
}
