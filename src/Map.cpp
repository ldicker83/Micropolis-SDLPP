// This file is part of Micropolis-SDLPP
// Micropolis-SDLPP is based on Micropolis
//
// Copyright © 2022 - 2026 Leeor Dicker
//
// Portions Copyright © 1989-2007 Electronic Arts Inc.
//
// Micropolis-SDLPP is free software; you can redistribute it and/or modify
// it under the terms of the GNU GPLv3, with additional terms. See the README
// file, included in this distribution, for details.
#include "Map.h"

#include "s_alloc.h"
#include "Sprite.h"
#include "Texture.h"
#include "Util.h"

#include "Math/Point.h"

#include <vector>

#include <SDL3/SDL.h>

extern SDL_Renderer* MainWindowRenderer;

extern Texture BigTileset;
extern Texture MainMapTexture;


namespace
{
	std::vector<int> MapBuffer;

	SDL_FRect TileDrawRect{ 0, 0, 16, 16 };
	bool Blink{ false };
};


MapData getMapData()
{
	return MapData
	{
		reinterpret_cast<const char*>(MapBuffer.data()),
		static_cast<unsigned int>(MapBuffer.size() * sizeof(int))
	};
}


int& tileValue(const Point<int>& location)
{
	return tileValue(location.x, location.y);
}


int& tileValue(const int x, const int y)
{
	return MapBuffer[static_cast<size_t>(x) * SimHeight + static_cast<size_t>(y)];
}


unsigned int maskedTileValue(const Point<int>& location)
{
	return maskedTileValue(location.x, location.y);
}


unsigned int maskedTileValue(const int x, const int y)
{
	return tileValue(x, y) & LowerMask;
}


unsigned int maskedTileValue(unsigned int tile)
{
	return tile & LowerMask;
}


bool tileIsPowered(const Point<int> coordinates)
{
	return tileIsPowered(tileValue(coordinates));
}


bool tileIsZoned(const Point<int> coordinates)
{
	return tileIsZoned(tileValue(coordinates));
}


bool tileIsRoad(const Point<int> coordinates)
{
	if (!coordinatesValid(coordinates))
	{
		return false;
	}

	return tileIsRoad(tileValue(coordinates));
}


bool tileCanBeBulldozed(const Point<int> coordinates)
{
	return tileCanBeBulldozed(tileValue(coordinates));
}


bool tileCanBurn(const Point<int> coordinates)
{
	return tileCanBurn(tileValue(coordinates));
}


bool tileIsPowered(int tile)
{
	return tile & PowerBit;
}


bool tileIsZoned(int tile)
{
	return tile & ZonedBit;
}


bool tileIsRoad(int tile)
{
	const auto maskedTile = maskedTileValue(tile);

	if (maskedTile < BridgeBase || maskedTile > RailLast)
	{
		return false;
	}

	if ((maskedTile >= PowerBase) && (maskedTile < RailHorizontalPowerVertical))
	{
		return false;
	}

	return true;
}


bool tileCanBeBulldozed(int tile)
{
	return tile & BulldozableBit;
}


bool tileCanBurn(int tile)
{
	return tile & BurnableBit;
}


void toggleBlinkFlag()
{
	Blink = !Blink;
}


void ResetMap()
{
	MapBuffer.resize(SimWidth * SimHeight);
	std::fill(MapBuffer.begin(), MapBuffer.end(), Dirt);
}


/**
 * Assumes \c begin and \c end are in a valid range
 */
void drawBigMapSegment(const Point<int>& begin, const Point<int>& end)
{
	SDL_SetRenderTarget(MainWindowRenderer, MainMapTexture.texture);

	SDL_FRect drawRect{ 0.0f, 0.0f, 16.0f, 16.0f };
	unsigned int tile = 0;

	for (int row = begin.x; row < end.x; row++)
	{
		for (int col = begin.y; col < end.y; col++)
		{
			tile = tileValue(row, col);
			// Blink lightning bolt in unpowered zone center
			if (Blink && tileIsZoned({ row, col }) && !tileIsPowered({ row, col }))
			{
				tile = LightningBolt;
			}

			drawRect = { row * drawRect.w, col * drawRect.h, drawRect.w, drawRect.h };

			const unsigned int masked = maskedTileValue(tile);
			TileDrawRect =
			{
				static_cast<float>((static_cast<int>(masked) % 32) * 16),
				static_cast<float>((static_cast<int>(masked) / 32) * 16),
				16.0f, 16.0f
			};

			SDL_RenderTexture(MainWindowRenderer, BigTileset.texture, &TileDrawRect, &drawRect);
		}
	}

	SDL_RenderPresent(MainWindowRenderer);
	SDL_SetRenderTarget(MainWindowRenderer, nullptr);
}


void drawBigMap()
{
	drawBigMapSegment(Point<int>{0, 0}, Point<int>{SimWidth, SimHeight});
}
