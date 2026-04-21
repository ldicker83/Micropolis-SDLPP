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
#include "Util.h"

#include "Budget.h"
#include "CityProperties.h"

#include "Map.h"

#include "w_tk.h"
#include "w_update.h"


#include "main.h"

#include <algorithm>
#include <array>
#include <format>
#include <locale>
#include <random>
#include <string>


namespace
{
    SimulationSpeed simulationSpeed;
    SimulationSpeed previousSimulationSpeed;

    std::array<std::string, 5> speedStringTable
    {
        "Paused", "Slow", "Normal", "Fast" , "African Swallow"
    };

    std::array<Vector<int>, 4> AdjacentVectorTable
    { {
        {  0, -1 },
        {  1,  0 },
        {  0,  1 },
        { -1,  0 },
	} };
};


/**
 * Set the locale for consistent number formatting.
 */
void setLocale()
{
    try
    {
        std::locale::global(std::locale("en_US.UTF-8"));
    }
    catch (const std::runtime_error&)
    {
        try
        {
            std::locale::global(std::locale("C"));
        }
        catch (...)
        {
        }
    }
}


const Vector<int> vectorFromPoints(const Point<int>& start, const Point<int>& end)
{
    return { end.x - start.x, end.y - start.y };
}


bool pointInRect(const Point<int>& point, const SDL_Rect& rect)
{
    return point.x >= rect.x && point.x < rect.x + rect.w && point.y >= rect.y && point.y < rect.y + rect.h;
}


bool pointInFRect(const Point<int>& point, const SDL_FRect& rect)
{
    return point.x >= rect.x && point.x < rect.x + rect.w && point.y >= rect.y && point.y < rect.y + rect.h;
}


SDL_FRect fRectFromRect(const SDL_Rect& rect)
{
    SDL_FRect fRect{};
    SDL_RectToFRect(&rect, &fRect);
    return fRect;
}


Point<int> positionToCell(const Point<int>& position, const Point<int>& offset)
{
    return
    {
        (((position.x) + (offset.x % 16)) / 16),
        (((position.y) + (offset.y % 16)) / 16),
    };
}


bool coordinatesValid(const Point<int>& position)
{
    return pointInRect(position, ValidMapCoordinates);
}


std::string numberToDollarDecimal(int value)
{
    return std::format("${:L}", value);
}


Vector<int> adjacentVector(SearchDirection direction)
{
	return AdjacentVectorTable.at(static_cast<int>(direction));
}


const std::string& speedString(SimulationSpeed speed)
{
    return speedStringTable[static_cast<int>(speed)];
}


void simSpeed(SimulationSpeed speed)
{
    if (speed == SimulationSpeed::Paused)
    {
        previousSimulationSpeed = simulationSpeed;
        simulationSpeed = speed;
        return;
    }

    simulationSpeed = speed;
}


SimulationSpeed simSpeed()
{
    return simulationSpeed;
}


void pause()
{
    previousSimulationSpeed = simulationSpeed;
    simSpeed(SimulationSpeed::Paused);
}


void resume()
{
    simSpeed(previousSimulationSpeed);
}


bool paused()
{
    return simulationSpeed == SimulationSpeed::Paused;
}


void setGameLevelFunds(int level, CityProperties& properties, Budget& budget)
{
    switch (level)
    {
    default:
    case 0:
        budget.CurrentFunds(20000);
        properties.GameLevel(0);
        break;

    case 1:
        budget.CurrentFunds(10000);
        properties.GameLevel(1);
        break;

    case 2:
        budget.CurrentFunds(5000);
        properties.GameLevel(2);
        break;
    }
}


static std::random_device RandomDevice;
static std::mt19937 PseudoRandomNumberGenerator(RandomDevice());


int randomRange(int min, int max)
{
    std::uniform_int_distribution<std::mt19937::result_type> prngDistribution(min, max);
    return prngDistribution(PseudoRandomNumberGenerator);
}


int random()
{
    return randomRange(0, std::mt19937::max());
}


int rand16()
{
    return randomRange(0, 32767) * (randomRange(0, 1) ? -1 : 1);
}
