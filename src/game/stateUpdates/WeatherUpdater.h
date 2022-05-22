#pragma once


#include "StateUpdater.h"
#include "Common.h"
#include <unordered_set>
#include <unordered_map>

class Game;

/*
 * Currently WeatherUpdater updates ONLY rain stuff
 * */

namespace world
{

	class WeatherUpdater : public StateUpdater
	{
	private:
		
		// All world's water tiles xDd quite dumb and inefficient
		// ->  find better way in the future!
		std::unordered_set<int> _waterTiles;

		// Mapping of all tiles, which are raining. 
		// Key = tile index val.first = ground moisture, val.second = cloud moisture xD
		std::unordered_map<int, std::pair<PK_ubyte, PK_ubyte>> _rainingTiles;

		int _dice_toRain = 20;
	public:

		WeatherUpdater(Game& gameRef, float updateCooldown, const std::unordered_set<int>& initialWaterTiles);
		~WeatherUpdater();

	protected:

		virtual void updateFunc();


	private:
		void floodToAdjacents(int tileIndex, uint64_t tileState);

	};
}
