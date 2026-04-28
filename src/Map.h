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

#include <array>

#include <SDL3/SDL.h>


/* Character Mapping */
constexpr auto Dirt = 0;
constexpr auto River = 2;
constexpr auto RiverEdge = 3;
constexpr auto RiverChannel = 4;
constexpr auto RiverEdgeFirst = 5;
constexpr auto RiverEdgeLast = 20;
constexpr auto TreeBase = 21;
constexpr auto TreeLast = 36;
constexpr auto Woods = 37;
constexpr auto Unused1 = 38;
constexpr auto Unused2 = 39; // \todo Rename this as it's not unused.
constexpr auto Woods2 = 40;
constexpr auto Woods3 = 41;
constexpr auto Woods4 = 42;
constexpr auto Woods1 = 43;
constexpr auto Rubble = 44;
constexpr auto RubbleLast = 47;
constexpr auto Flood = 48;
constexpr auto FloodLast = 51;
constexpr auto RadiationTile = 52; // Radiation tile
constexpr auto Unused3 = 53;
constexpr auto Unused4 = 54;
constexpr auto Unused5 = 55;
constexpr auto FireBase = 56;
constexpr auto FireLast = 63; // Fire end value
constexpr auto BridgeBase = 64; // Road base value
constexpr auto BridgeHorizontal = 64;
constexpr auto BridgeVertical = 65;
constexpr auto RoadHorizontal = 66;
constexpr auto RoadVertical = 67;
constexpr auto RoadIntersection = 76;
constexpr auto RoadPowerHorizontal = 77;
constexpr auto RoadPowerVertical = 78;
constexpr auto BRWH = 79;
constexpr auto TrafficLightBase = 80; // Light traffic base value
constexpr auto BRWV = 95;
constexpr auto BRWXXX1 = 111;
constexpr auto BRWXXX2 = 127;
constexpr auto BRWXXX3 = 143;
constexpr auto TrafficHeavyBase = 144; // Heavy traffic base value
constexpr auto BRWXXX4 = 159;
constexpr auto BRWXXX5 = 175;
constexpr auto BRWXXX6 = 191;
constexpr auto RoadLast = 206;
constexpr auto BRWXXX7 = 207;
constexpr auto PowerBase = 208;
constexpr auto PowerHorizontalWater = 208;
constexpr auto PowerVerticalWater = 209;
constexpr auto PowerHorizontal = 210;
constexpr auto PowerVertical = 211;
constexpr auto RailHorizontalPowerVertical = 221;
constexpr auto RailVerticalPowerHorizontal = 222;
constexpr auto PowerLast = 222;
constexpr auto Unused6 = 223; // \todo Rename this as it's not unused.
constexpr auto RailBase = 224;
constexpr auto RailWaterHorizontal = 224;
constexpr auto RailWaterVertical = 225;
constexpr auto RailHorizontal = 226;
constexpr auto RailVertical = 227;
constexpr auto RailHorizontalRoadVertical = 237;
constexpr auto RailVerticalRoadHorizontal = 238;
constexpr auto RailLast = 238;
constexpr auto RoadVerticalPowerHorizontal = 239;
constexpr auto ResidentialBase = 240;
constexpr auto ResidentialEmpty = 244;
constexpr auto House = 249;
constexpr auto LHTHR = 249;
constexpr auto HHTHR = 260;
constexpr auto ResidentialZoneBase = 265;
constexpr auto Hospital = 409;
constexpr auto Church = 418;
constexpr auto CommercialBase = 423;
constexpr auto CommercialEmpty = 427;
constexpr auto CommercialZoneBase = 436;
constexpr auto IndustryBase = 612;
constexpr auto IndustryEmpty = 616;
constexpr auto IndutryLast = 620;
constexpr auto IND1 = 621;
constexpr auto IndustrialZoneBase = 625;
constexpr auto IND2 = 641;
constexpr auto IND3 = 644;
constexpr auto IND4 = 649;
constexpr auto IND5 = 650;
constexpr auto IND6 = 676;
constexpr auto IND7 = 677;
constexpr auto IND8 = 686;
constexpr auto IND9 = 689;
constexpr auto PortBase = 693; // Port base value
constexpr auto Port = 698;
constexpr auto PortLast = 708;
constexpr auto AirportBase = 709;
constexpr auto Radar = 711;
constexpr auto Airport = 716;
constexpr auto CoalPowerBase = 745;
constexpr auto PowerPlant = 750;
constexpr auto PowerPlantLast = 760;
constexpr auto FireStationBase = 761;
constexpr auto FireStation = 765;
constexpr auto PoliceStationBase = 770;
constexpr auto PoliceStation = 774;
constexpr auto StadiumBase = 779;
constexpr auto Stadium = 784;
constexpr auto StatdiumFull = 800;
constexpr auto NuclearPowerBase = 811;
constexpr auto NuclearPower = 816;
constexpr auto ZoneLast = 826;
constexpr auto LightningBolt = 827;
constexpr auto HBRDG0 = 828;
constexpr auto HBRDG1 = 829;
constexpr auto HBRDG2 = 830;
constexpr auto HBRDG3 = 831;
constexpr auto Radar0 = 832;
constexpr auto Radar1 = 833;
constexpr auto Radar2 = 834;
constexpr auto Radar3 = 835;
constexpr auto Radar4 = 836;
constexpr auto Radar5 = 837;
constexpr auto Radar6 = 838;
constexpr auto Radar7 = 839;
constexpr auto Fountain = 840;
constexpr auto INDBASE2 = 844;
constexpr auto TELEBASE = 844;
constexpr auto TELELAST = 851;
constexpr auto SmokeBase = 852;
constexpr auto ExplosionTiny = 860;
constexpr auto ExplosionTinySome = 864; // The hell does this name mean??
constexpr auto ExplosionTinyLast = 867;
constexpr auto CoalPowerSmoke1 = 916;
constexpr auto CoalPowerSmoke2 = 920;
constexpr auto CoalPowerSmoke3 = 924;
constexpr auto CoalPowerSmoke4 = 928;
constexpr auto FootballGame1 = 932;
constexpr auto FootballGame2 = 940;
constexpr auto VBRDG0 = 948;
constexpr auto VBRDG1 = 949;
constexpr auto VBRDG2 = 950;
constexpr auto VBRDG3 = 951;

constexpr auto TILE_COUNT = 960;


/* Status Bits */
constexpr int PowerBit = 0x8000; /* 20 bit 15 */
constexpr int ConductiveBit = 0x4000; /* 10 bit 14 */
constexpr int BurnableBit = 0x2000; /* 8 bit 13 */
constexpr int BulldozableBit = 0x1000; /* 4 bit 12 */
constexpr int AnimatedBit = 0x0800;
constexpr int ZonedBit = 0x0400; /* 1 bit 10 */
constexpr int UpperMask = 0xFC00; /* mask for upper 6 bits	*/
constexpr int LowerMask = 0x03FF; /* mask for low 10 bits	*/

constexpr int BulldozableBurnableBits = BulldozableBit + BurnableBit;
constexpr int BulldozableBurnableConductiveBits = BulldozableBit + BurnableBit + ConductiveBit;
constexpr int BurnableConductiveBits = BurnableBit + ConductiveBit;


/* Constants */
constexpr auto SimWidth = 120;
constexpr auto SimHeight = 100;
constexpr SDL_Rect ValidMapCoordinates{ 0, 0, SimWidth - 1, SimHeight - 1 };

constexpr auto HalfWorldWidth = SimWidth / 2;
constexpr auto HalfWorldHeight = SimHeight / 2;

constexpr auto QuarterWorldWidth = SimWidth / 4;
constexpr auto QuarterWorldHeight = SimHeight / 4;

constexpr auto EighthWorldWidth = SimWidth / 8;
constexpr auto EighthWorldHeight = (SimHeight + 7) / 8;


/**
 * Struct used for file i/o
 */
struct MapData
{
	const char* data;
	const size_t size;
};

MapData getMapData();

int& tileValue(const Point<int>& location);
int& tileValue(const int x, const int y);

unsigned int maskedTileValue(const Point<int>& location);
unsigned int maskedTileValue(const int x, const int y);
unsigned int maskedTileValue(unsigned int tile);

bool tileIsPowered(const Point<int> coordinates);
bool tileIsZoned(const Point<int> coordinates);
bool tileIsRoad(const Point<int> coordinates);
bool tileCanBeBulldozed(const Point<int> coordinates);
bool tileCanBurn(const Point<int> coordinates);

bool tileIsPowered(int tile);
bool tileIsZoned(int tile);
bool tileIsRoad(int tile);
bool tileCanBeBulldozed(int tile);
bool tileCanBurn(int tile);

void toggleBlinkFlag();

void ResetMap();

void drawBigMapSegment(const Point<int>& begin, const Point<int>& end);
void drawBigMap();
