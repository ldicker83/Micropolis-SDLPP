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
#pragma once

#include "Math/Point.h"
#include "Math/Vector.h"

#include <SDL3/SDL.h>

#include <string>
#include <unordered_map>

class Budget;
class CityProperties;

enum class SimulationSpeed
{
	Paused,
	Slow,
	Normal,
	Fast,
	AfricanSwallow
};


enum class SearchDirection
{
	Up, Right, Down, Left, Undefined
};


static const std::unordered_map<SearchDirection, Vector<int>> SearchDirectionVectors
{
	{ SearchDirection::Up, { 0, -1 } },
	{ SearchDirection::Right, { 1, 0 } },
	{ SearchDirection::Down, { 0, 1 } },
	{ SearchDirection::Left, { -1, 0 } }
};

void setLocale();

Vector<int> adjacentVector(SearchDirection direction);

const std::string& speedString(SimulationSpeed speed);

std::string numberToDollarDecimal(int value);

void simSpeed(SimulationSpeed speed);
SimulationSpeed simSpeed();

void pause();
void resume();

bool paused();

void setGameLevelFunds(int level, CityProperties& properties, Budget&);
bool coordinatesValid(const Point<int>& position);
Point<int> positionToCell(const Point<int>& position, const Point<int>& offset);
const Vector<int> vectorFromPoints(const Point<int>& start, const Point<int>& end);
bool pointInRect(const Point<int>& point, const SDL_Rect& rect);
bool pointInFRect(const Point<int>& point, const SDL_FRect& rect);

SDL_FRect fRectFromRect(const SDL_Rect& rect);

int longestAxis(const Vector<int>& vec);

int randomRange(int min, int max);
int random();
int rand16();
