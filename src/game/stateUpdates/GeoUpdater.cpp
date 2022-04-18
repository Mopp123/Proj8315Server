
#include "GeoUpdater.h"
#include "game/Game.h"
#include "Debug.h"
#include "game/world/Tile.h"

#include <algorithm>

namespace world
{
	GeoUpdater::GeoUpdater(Game& gameRef, float updateCooldown) : 
		StateUpdater(gameRef, updateCooldown)
	{
		// Just as test randomize some tectonic plate edges..
		//_tectonicPlateEdges.push_back(5 + 5 * GAME_WORLD_WIDTH);

		// testing some tectonic plate edge drawing...
		drawEdgeLine(5,1, TileStateDirection::TILE_STATE_dirS);
		drawEdgeLine(1,30, TileStateDirection::TILE_STATE_dirE);

		drawEdgeLine(35, 1, TileStateDirection::TILE_STATE_dirSE);
		drawEdgeLine(56, 1, TileStateDirection::TILE_STATE_dirSW);
		
		drawEdgeLine(1, 62, TileStateDirection::TILE_STATE_dirNE);
		
		Debug::log("GeoUpdater initialized");
	}
	
	GeoUpdater::~GeoUpdater()
	{
	}

        void GeoUpdater::updateFunc()
	{
		tryTriggerEdgeActive();
		
		// Try alter some vulcanicly active tile's elevation..
		size_t vulcActiveLen = _vulcaniclyActive.size();
		Debug::log("Vulcanicly active: " + std::to_string(vulcActiveLen));
		if(vulcActiveLen > 0)
		{
			int randFromVulcList = std::rand() % (int)vulcActiveLen;
			int vulcTile = _vulcaniclyActive[randFromVulcList];
			int diceThrow = std::rand() % 20;
			if(diceThrow > 18)
			{
				alterElevation(vulcTile, true);
				// If elevation gets increased -> this might bleed to surrounding tiles
				int bleedDiceThrow = std::rand() % 10;
				if(bleedDiceThrow > 2)
				{
					bleedToAdjacents(vulcTile);
				}

			}
		}
	}

	void GeoUpdater::tryTriggerEdgeActive()
	{
		
		// Try to set some tile to vulcanicly active state..
		int randEdgeTile = std::rand() % _tectonicPlateEdges.size();
		int randHorizontal = (std::rand() % 3) - 1;
		int randVertMod = std::rand() % 3;
		int randVertical = 0;
		if(randVertMod == 1)
			randVertical = GAME_WORLD_WIDTH;
		else if(randVertMod == 2)
			randVertical = -GAME_WORLD_WIDTH;

		int toTriggerTile = _tectonicPlateEdges[randEdgeTile] + randHorizontal + randVertical;
		
		int diceThrow = std::rand() % 20;
		if(diceThrow > 16)
		{
			uint64_t tileState = _gameRef.getTileState(toTriggerTile);
			if(get_tile_terrelevation(tileState) < TILE_STATE_MAX_terrElevation)
			{
				// Dont add the same tile to this list
				if(std::find(_vulcaniclyActive.begin(), _vulcaniclyActive.end(), toTriggerTile) == _vulcaniclyActive.end())
					_vulcaniclyActive.push_back(toTriggerTile);
			}
		}
	}

	void GeoUpdater::alterElevation(int tileIndex, bool enableBleed)
	{

		if(tileIndex < 0 || tileIndex >= GAME_WORLD_WIDTH * GAME_WORLD_WIDTH)
			return;

		//Debug::log("altering tile at: " + std::to_string(tileIndex));
		
		uint64_t tileState = _gameRef.getTileState(tileIndex);
		PK_byte currentElevation = get_tile_terrelevation(tileState);
		if(currentElevation < TILE_STATE_MAX_terrElevation)
		{

			set_tile_terrelevation(tileState, currentElevation+1);
			set_tile_terrtype(tileState, (PK_byte)TileStateTerrType::TILE_STATE_terrTypeCommonVulcanic);
			_gameRef.setTileState(tileIndex, tileState);

			// Throw dice, do we set this back to non active
			int diceThrow = std::rand() % TILE_STATE_MAX_terrElevation - currentElevation + 1;
			if(diceThrow <= 0)
			{
				set_tile_terrtype(tileState, (PK_byte)TileStateTerrType::TILE_STATE_terrTypeCommonDeadland);
				_gameRef.setTileState(tileIndex, tileState);
				auto iter = std::find(_vulcaniclyActive.begin(), _vulcaniclyActive.end(), tileIndex);
				if(iter != _vulcaniclyActive.end())
					_vulcaniclyActive.erase(iter);
			}
		}
		else if(enableBleed)
		{
		
			// Throw dice, do we bleed..
			int diceThrow = std::rand() % TILE_STATE_MAX_terrElevation - currentElevation + 1;
			if(diceThrow <= 0)
			{
				// *NOTE! WARNIING! This caused segmentation fault.. propably too deep recursion rabbit hole...
				bleedToAdjacents(tileIndex);
			}
		}
	}

	void GeoUpdater::bleedToAdjacents(int tileIndex)
	{
		int adjacentTiles[8];
		adjacentTiles[0] = tileIndex - 1;
		adjacentTiles[2] = tileIndex + 1;
		adjacentTiles[1] = tileIndex - GAME_WORLD_WIDTH;
		adjacentTiles[3] = tileIndex + GAME_WORLD_WIDTH;
		
		adjacentTiles[4] = tileIndex - GAME_WORLD_WIDTH - 1;
		adjacentTiles[5] = tileIndex - GAME_WORLD_WIDTH + 1;
		adjacentTiles[6] = tileIndex + GAME_WORLD_WIDTH - 1;
		adjacentTiles[7] = tileIndex + GAME_WORLD_WIDTH + 1;
		for(int i = 0; i < 4; ++i)
			alterElevation(adjacentTiles[i], false);
	}

	
	void GeoUpdater::drawEdgeLine(int startX, int startY, TileStateDirection startDir)
	{
		int currentX = startX;
		int currentY = startY;
		int currentDir = (int)startDir;
		while(currentX > 0 && currentX < GAME_WORLD_WIDTH - 1 && currentY > 0 && currentY < GAME_WORLD_WIDTH - 1)
		{
			// Mark current tile as tectonic edge
			// First prevent adding multiple times
			int currentTileIndex = currentX + currentY * GAME_WORLD_WIDTH;
			if(std::find(_tectonicPlateEdges.begin(), _tectonicPlateEdges.end(), currentTileIndex) == _tectonicPlateEdges.end())
				_tectonicPlateEdges.push_back(currentTileIndex);


			// moving forward..
			int randomDir = (std::rand() % 3) - 1;
			currentDir += randomDir;
			if(currentDir < 0) currentDir = 7;
			// mod with 7 cuz 7 directions.. wrap back around to 0 after 7(7 is NW and N is 0)
			currentDir = currentDir % 8;
			switch((TileStateDirection)currentDir)
			{
				case TileStateDirection::TILE_STATE_dirN:
					currentY -= 1; 
					break;
				
				case TileStateDirection::TILE_STATE_dirNE:
					currentX += 1; 
					currentY -= 1; 
					break;
				
				case TileStateDirection::TILE_STATE_dirE:
					currentX += 1; 
					break;
				
				case TileStateDirection::TILE_STATE_dirSE:
					currentX += 1; 
					currentY += 1; 
					break;
				
				case TileStateDirection::TILE_STATE_dirS:
					currentY += 1; 
					break;

				case TileStateDirection::TILE_STATE_dirSW:
					currentX -= 1; 
					currentY += 1; 
					break;

				case TileStateDirection::TILE_STATE_dirW:
					currentX -= 1; 
					break;
				
				case TileStateDirection::TILE_STATE_dirNW:
					currentX -= 1; 
					currentY -= 1; 
					break;
				
				default:
					Debug::log("ERROR");
					break;
			}
		}
	}
}
