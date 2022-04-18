#pragma once

#include "StateUpdater.h"
#include "game/world/Tile.h"
#include <vector>

class Game;

namespace world
{
	class GeoUpdater : public StateUpdater
	{
	private:
		
		// Tile IDs which are tectonic plates' edges
		std::vector<int> _tectonicPlateEdges;
		// Tile IDs which are vulcanicly active
		std::vector<int> _vulcaniclyActive;
		
	public:
		GeoUpdater(Game& gameRef, float updateCooldown);
		~GeoUpdater();
	protected:
		virtual void updateFunc();

	private:
		// Attempts to trigger some near edge tile to vulcanicly active state..
		void tryTriggerEdgeActive();
		// Alters tile's elevation and sets its terrain type to "vulcanic"
		void alterElevation(int tileIndex, bool enableBleed = false);
		// Alters tile's surrounding tiles' elevation
		void bleedToAdjacents(int tileIndex);

		void drawEdgeLine(int startX, int startY, TileStateDirection startDir);
	};
};
