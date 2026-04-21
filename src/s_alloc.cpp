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
#include "s_alloc.h"

#include "EffectMap.h"
#include "Map.h"
#include "Power.h"

#include "Util.h"

#include <array>
#include <map>
#include <vector>


/* Allocate Stuff */

Point<int> SimulationTarget{};

int CurrentTile; // unmasked tile value
int CurrentTileMasked; // masked tile value

int RoadCount, RailCount, BurningTileCount;

int ResidentialPopulationCount, CommercialPopulationCount, IndustrialPopulationCount, PopulationTotal, PreviousPopulationTotal;
int ResidentialZoneCount, CommercialZoneCount, IndustrialZoneCount, CombinedZoneCount;
int HospitalCount, ChurchCount, StadiumCount;
int PoliceStationCount, FireStationCount;
int CoalPowerPlantCount, NuclearPowerPlantCount, SeaPortCount, AirportCount;

int HospitalBuildCount, ChurchBuildCount;
int CrimeAverage, PolluteAverage, LVAverage;

int CityTime;
int StartingYear;

int ResidentialPopulationHistoryHighest;
int CommercialPopulationHistoryHighest;
int IndustrialPopulationHistoryHighest;

int RoadEffect, PoliceEffect, FireEffect;


EffectMap PopulationDensityMap({HalfWorldWidth, HalfWorldHeight });
EffectMap TrafficDensityMap({ HalfWorldWidth, HalfWorldHeight });
EffectMap PollutionMap({ HalfWorldWidth, HalfWorldHeight });
EffectMap LandValueMap({ HalfWorldWidth, HalfWorldHeight });
EffectMap CrimeMap({ HalfWorldWidth, HalfWorldHeight });

EffectMap TerrainMem({ QuarterWorldWidth, QuarterWorldHeight });

EffectMap RateOfGrowthMap({ EighthWorldWidth, EighthWorldHeight });
EffectMap FireStationMap({ EighthWorldWidth, EighthWorldHeight });
EffectMap PoliceStationMap({ EighthWorldWidth, EighthWorldHeight });

EffectMap PoliceProtectionMap({ EighthWorldWidth, EighthWorldHeight });
EffectMap FireProtectionMap({ EighthWorldWidth, EighthWorldHeight });

EffectMap ComRate({ EighthWorldWidth, EighthWorldHeight });

GraphHistory ResidentialPopulationHistory{};
GraphHistory CommercialPopulationHistory{};
GraphHistory IndustrialPopulationHistory{};

GraphHistory MoneyHis{};
GraphHistory PollutionHistory{};
GraphHistory CrimeHistory{};
GraphHistory MiscHistory{};

GraphHistory ResHis120Years{};
GraphHistory ComHis120Years{};
GraphHistory IndHis120Years{};

GraphHistory MoneyHis120Years{};
GraphHistory PollutionHis120Years{};
GraphHistory CrimeHis120Years{};
GraphHistory MiscHis120Years{};

namespace
{
    const std::map<SearchDirection, Vector<int>> AdjacentVector
    {
        { SearchDirection::Left,  { -1,  0 } },
        { SearchDirection::Right, {  1,  0 } },
        { SearchDirection::Up,    {  0, -1 } },
        { SearchDirection::Down,  {  0,  1 } },
        { SearchDirection::Undefined, {} }
    };

    void resetHalfArrays()
    {
        PopulationDensityMap.reset();
        TrafficDensityMap.reset();
        PollutionMap.reset();
        LandValueMap.reset();
        CrimeMap.reset();
    }

    void resetQuarterArrays()
    {
        TerrainMem.reset();
    }

    void resetHistoryArrays()
    {
        ResidentialPopulationHistory.fill(0);
        CommercialPopulationHistory.fill(0);
        IndustrialPopulationHistory.fill(0);

        MoneyHis.fill(0);
        PollutionHistory.fill(0);
        CrimeHistory.fill(0);
        MiscHistory.fill(0);

        ResHis120Years.fill(0);
        ComHis120Years.fill(0);
        IndHis120Years.fill(0);

        MoneyHis120Years.fill(0);
        PollutionHis120Years.fill(0);
        CrimeHis120Years.fill(0);
        MiscHis120Years.fill(0);

        MiscHistory.fill(0);
        resetPowerMap();
    }
};


void initMapArrays()
{
    resetHalfArrays();
    resetQuarterArrays();
    resetHistoryArrays();
}


bool moveSimulationTarget(SearchDirection direction)
{
    const Point<int> newTargetCoordinates{ SimulationTarget + AdjacentVector.at(direction) };

    if (!coordinatesValid(newTargetCoordinates))
    {
        return false;
    }

    SimulationTarget += AdjacentVector.at(direction);
    return true;
}
