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

#include "GameOptions.h"
#include "Texture.h"

#include "Math/Point.h"

#include <string>

#include <SDL3/SDL.h>

 /* Constants */
extern int CurrentTile; // unmasked tile value
extern int CurrentTileMasked; // masked tile value

extern int RoadCount, RailCount, BurningTileCount;
extern int ResidentialPopulationCount, CommercialPopulationCount, IndustrialPopulationCount, PopulationTotal, PreviousPopulationTotal;
extern int ResidentialZoneCount, CommercialZoneCount, IndustrialZoneCount, CombinedZoneCount;
extern int HospitalCount, ChurchCount, StadiumCount;
extern int PoliceStationCount, FireStationCount;
extern int CoalPowerPlantCount, NuclearPowerPlantCount, SeaPortCount, AirportCount;
extern int HospitalBuildCount, ChurchBuildCount;
extern int CrimeAverage, PolluteAverage, LVAverage;

extern int StartingYear;
extern int CityTime;
extern int ScenarioID;
extern int ShakeNow;

extern int RoadEffect, PoliceEffect, FireEffect;

extern int DisasterEvent;
extern int DisasterWait;

extern int ResCap, ComCap, IndCap;
extern int PoweredZoneCount;
extern int UnpoweredZoneCount;

extern int ScoreType;
extern int ScoreWait;

extern int InitSimLoad;
extern int DoInitialEval;

extern SDL_Renderer* MainWindowRenderer;

void showBudgetWindow();

void initWillStuff();

GameOptions& gameplayOptions();

const Point<int>& viewOffset();
