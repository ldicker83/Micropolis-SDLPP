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

#include "EffectMap.h"
#include "main.h"
#include "Util.h"

#include "Math/Point.h"


#include <array>


constexpr auto HistoryLength = 120;


extern Point<int> SimulationTarget;


// 2X2 Maps  60 x 50
extern EffectMap PopulationDensityMap;
extern EffectMap TrafficDensityMap;
extern EffectMap PollutionMap;
extern EffectMap LandValueMap;
extern EffectMap CrimeMap;

// 4X4 Maps  30 x 25
extern EffectMap TerrainMem;

/* 8X8 Maps  15 x 13 */
extern EffectMap RateOfGrowthMap;
extern EffectMap FireStationMap;
extern EffectMap PoliceStationMap;

extern EffectMap PoliceProtectionMap;
extern EffectMap FireProtectionMap;

extern EffectMap ComRate;

using GraphHistory = std::array<int, HistoryLength>;

extern GraphHistory ResidentialPopulationHistory;
extern GraphHistory CommercialPopulationHistory;
extern GraphHistory IndustrialPopulationHistory;
extern GraphHistory MoneyHis;
extern GraphHistory PollutionHistory;
extern GraphHistory CrimeHistory;
extern GraphHistory MiscHistory;

extern GraphHistory ResHis120Years;
extern GraphHistory ComHis120Years;
extern GraphHistory IndHis120Years;
extern GraphHistory MoneyHis120Years;
extern GraphHistory PollutionHis120Years;
extern GraphHistory CrimeHis120Years;
extern GraphHistory MiscHis120Years;

extern int ResidentialPopulationHistoryHighest;
extern int CommercialPopulationHistoryHighest;
extern int IndustrialPopulationHistoryHighest;

void initMapArrays();
bool moveSimulationTarget(SearchDirection direction);
