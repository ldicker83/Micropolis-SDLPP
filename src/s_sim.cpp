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
#include "s_sim.h"

#include "Budget.h"
#include "CityProperties.h"
#include "Constants.h"
#include "Evaluation.h"

#include "Map.h"

#include "Power.h"

#include "RCI.h"

#include "s_alloc.h"
#include "s_disast.h"
#include "s_msg.h"

#include "Util.h"

#include "Scan.h"
#include "Sprite.h"
#include "Traffic.h"
#include "Zone.h"

#include "Math/Point.h"


#include <SDL3/SDL.h>

#include <algorithm>
#include <iostream>


/* Simulation */

constexpr auto CensusRate = 4;
constexpr auto TaxFrequency = 48;

int CrimeRamp, PolluteRamp ;
int ResCap, ComCap, IndCap;
float EMarket = 4.0;
int DisasterEvent;
int DisasterWait;
int ScoreType;
int ScoreWait;
int PoweredZoneCount;
int UnpoweredZoneCount;
int AvCityTax;
int Scycle = 0;
int Fcycle = 0;
int DoInitialEval = 0;
int MeltX, MeltY;


namespace
{
	RCI rci;

	/**
	 * Fire Protection Thresholds
	 * 0 = No Fire Protection
	 * 1 = Minimal fire protection (at least one fire station within range)
	 * 21 = Good coverage (fire station within range of the tile)
	 * 101 = Excellent coverage (fire station very close to the tile)
	 */
	static constexpr std::array<int, 4> FireProtectionThresholds = { 0, 1, 21, 101 };

	/**
	 * Fire Burnout Rates
	 * 10 (no protection) = 10% chance of burning out each cycle
	 * 3 (minimal protection) = ~33% chance of burning out each cycle
	 * 2 (good coverage) = ~50% chance of burning out each cycle
	 * 1 (excellent coverage) = 100% chance of burning out each cycle
	 */
	static constexpr std::array<int, 4> BurnoutRates = { 10, 3, 2, 1 };

	/**
	 * Convert fire protection level to burnout rate index
	 * Uses binary search to find the appropriate threshold tier
	 */
	int getBurnoutRateIndex(int fireProtection)
	{
		auto thresholdIt = std::upper_bound(FireProtectionThresholds.begin(), FireProtectionThresholds.end(), fireProtection);
        return static_cast<int>(std::distance(FireProtectionThresholds.begin(), thresholdIt) - 1);
	}


    int getBurnoutRate(int fireProtection)
    {
        return BurnoutRates[getBurnoutRateIndex(fireProtection)];
	}


    void tryFireBurnout(const Point<int>& position)
    {
        int fireProtection = FireProtectionMap.value(position.skewInverseBy({ 8, 8 }));
        if (!randomRange(0, getBurnoutRate(fireProtection)))
        {
            tileValue(position.x, position.y) = Rubble + randomRange(0, 3) | BulldozableBit;
        }
	}


	void explodeIfIndustrial(const Point<int>& position, int tile)
    {
        const auto maskedValue = maskedTileValue(tile);
        if (maskedValue > IndustrialZoneBase && maskedValue <= PortLast)
        {
            makeExplosionAt(position.skewBy({ 16, 16 }));
        }
    }


	void propagateFireTo(const Point<int>& position)
    {
        const int tile = tileValue(position);
        if (tileCanBurn(tile))
        {
            if (tileIsZoned(tile))
            {
                condemnZone(position.x, position.y, tile);
				explodeIfIndustrial(position, tile);
            }

            tileValue(position) = (FireBase + randomRange(0, 3)) | AnimatedBit;
        }
    }
}


void DoFire()
{
    for (const auto [direction, vector] : SearchDirectionVectors)
    {
        // 12.5% chance to continue propagation in this direction
		if ((rand16() & 7))
        {
           continue;
        }

		const auto adjacentPosition = SimulationTarget + vector;
        if(coordinatesValid(adjacentPosition))
        {
            propagateFireTo(adjacentPosition);
		}
    }

    tryFireBurnout(SimulationTarget);
}


void DoAirport()
{
    if (!(randomRange(0, 5)))
    {
        generateAirplane(SimulationTarget);
        return;
    }
    if (!(randomRange(0, 12)))
    {
        generateHelicopter(SimulationTarget);
    }
}


void DoMeltdown(const int x, const int y)
{
    generateExplosion({ x, y });

    for (int row = (x - 1); row < (x + 3); ++row)
    {
        for (int col = (y - 1); col < (y + 3); ++col)
        {
            tileValue(row, col) = FireBase + randomRange(0, 3) | AnimatedBit;
        }
    }

    for (int i = 0; i < 200; ++i)
    {
        const int radiationX = x - 20 + randomRange(0, 40);
        const int radiationY = y - 15 + randomRange(0, 30);

        if (!coordinatesValid({ radiationX, radiationY }))
        {
            continue;
        }

        const int tile = tileValue(radiationX, radiationY);

        if (tile & ZonedBit)
        {
            continue;
        }

        if ((tile & BurnableBit) || tile == Dirt)
        {
            tileValue(radiationX, radiationY) = RadiationTile;
        }
    }

    ClearMes();
    SendMesAt(NotificationId::NuclearMeltdownReported, x, y);
}


void DoRail(const Point<int>& position)
{
    RailCount++;
    generateTrain(position);
   
    if (RoadEffect < 30) // Deteriorating  Rail
    {
        if (randomRange(0, 511) == 0)
        {
            const unsigned int tile = tileValue(position.x, position.y);
            if (!(tile & ConductiveBit))
            {
                if (RoadEffect < randomRange(0, 31))
                {
                    if (maskedTileValue(tile) < (RailBase + 2))
                    {
                        tileValue(position.x, position.y) = River;
                    }
                    else
                    {
                        tileValue(position.x, position.y) = Rubble + randomRange(0, 3) | BulldozableBit;
                    }
                    return;
                }
            }
        }
    }
}


void DoRadTile()
{
    if (randomRange(0, 4095) == 0) // Radioactive decay
    {
        tileValue(SimulationTarget.x, SimulationTarget.y) = Dirt;
    }
}


int GetBoatDis()
{
    int dist = 99999;
    //int mx = (SimulationTarget.x * 16) + 8;
    //int my = (SimulationTarget.y * 16) + 8;

    /*
    for (SimSprite* sprite = sim->sprite; sprite != NULL; sprite = sprite->next)
    {
        if ((sprite->type == SHI) && (sprite->frame != 0))
        {
            if ((dx = sprite->x + sprite->x_hot - mx) < 0)
            {
                dx = -dx;
            }
            if ((dy = sprite->y + sprite->y_hot - my) < 0)
            {
                dy = -dy;
            }

            dx += dy;

            if (dx < dist)
            {
                dist = dx;
            }
        }
    }
    */

    return dist;
}


bool DoBridge()
{
  static int HDx[7] = { -2,  2, -2, -1,  0,  1,  2 };
  static int HDy[7] = { -1, -1,  0,  0,  0,  0,  0 };
  static int HBRTAB[7] = {
    HBRDG1 | BulldozableBit, HBRDG3 | BulldozableBit, HBRDG0 | BulldozableBit,
    River, BRWH | BulldozableBit, River, HBRDG2 | BulldozableBit };
  static int HBRTAB2[7] = {
    River, River, BridgeHorizontal | BulldozableBit, BridgeHorizontal | BulldozableBit, BridgeHorizontal | BulldozableBit,
    BridgeHorizontal | BulldozableBit, BridgeHorizontal | BulldozableBit };
  static int VDx[7] = {  0,  1,  0,  0,  0,  0,  1 };
  static int VDy[7] = { -2, -2, -1,  0,  1,  2,  2 };
  static int VBRTAB[7] = {
    VBRDG0 | BulldozableBit, VBRDG1 | BulldozableBit, River, BRWV | BulldozableBit,
    River, VBRDG2 | BulldozableBit, VBRDG3 | BulldozableBit };
  static int VBRTAB2[7] = {
    BridgeVertical | BulldozableBit, River, BridgeVertical | BulldozableBit, BridgeVertical | BulldozableBit,
    BridgeVertical | BulldozableBit, BridgeVertical | BulldozableBit, River };
  int z, x, y, MPtem;

  if (CurrentTileMasked == BRWV) { /*  Vertical bridge close */
    if ((!(rand16() & 3)) &&
	(GetBoatDis() > 340))
      for (z = 0; z < 7; z++) { /* Close  */
	x = SimulationTarget.x + VDx[z];
	y = SimulationTarget.y + VDy[z];
	if (coordinatesValid({ x, y }))
	  if ((tileValue(x, y) & LowerMask) == (VBRTAB[z] & LowerMask))
	    tileValue(x, y) = VBRTAB2[z];
      }
    return true;
  }
  if (CurrentTileMasked == BRWH) { /*  Horizontal bridge close  */
    if ((!(rand16() & 3)) &&
	(GetBoatDis() > 340))
      for (z = 0; z < 7; z++) { /* Close  */
	x = SimulationTarget.x + HDx[z];
	y = SimulationTarget.y + HDy[z];
	if (coordinatesValid({ x, y }))
	  if ((tileValue(x, y) & LowerMask) == (HBRTAB[z] & LowerMask))
	    tileValue(x, y) = HBRTAB2[z];
      }
    return true;
  }

  if ((GetBoatDis() < 300) || (!(rand16() & 7))) {
    if (CurrentTileMasked & 1) {
      if (SimulationTarget.x < (SimWidth - 1))
	if (tileValue(SimulationTarget.x + 1, SimulationTarget.y) == RiverChannel) { /* Vertical open */
	  for (z = 0; z < 7; z++) {
	    x = SimulationTarget.x + VDx[z];
	    y = SimulationTarget.y + VDy[z];
	    if (coordinatesValid({ x, y }))  {
	      MPtem = tileValue(x, y);
	      if ((MPtem == RiverChannel) ||
		  ((MPtem & 15) == (VBRTAB2[z] & 15)))
		tileValue(x, y) = VBRTAB[z];
	    }
	  }
	  return true;
	}
      return false;
    } else {
      if (SimulationTarget.y > 0)
	if (tileValue(SimulationTarget.x, SimulationTarget.y - 1) == RiverChannel) { /* Horizontal open  */
	  for (z = 0; z < 7; z++) {
	    x = SimulationTarget.x + HDx[z];
	    y = SimulationTarget.y + HDy[z];
	    if (coordinatesValid({ x, y })) {
	      MPtem = tileValue(x, y);
	      if (((MPtem & 15) == (HBRTAB2[z] & 15)) ||
		  (MPtem == RiverChannel))
		tileValue(x, y) = HBRTAB[z];
	    }
	  }
	  return true;
	}
      return false;
    }
  }
  return false;
}


void DoRoad()
{
    static int DensityTable[3] =
    {
        BridgeBase,   // No Traffic
        TrafficLightBase,   // Light Traffic
        TrafficHeavyBase    // Heavy Traffic
    };

    RoadCount++;

    if (RoadEffect < 30) // Deteriorating Roads
    {
        if (!(rand16() & 511))
        {
            if (!(CurrentTile & ConductiveBit))
            {
                if (RoadEffect < (rand16() & 31))
                {
                    if (((CurrentTileMasked & 15) < 2) || ((CurrentTileMasked & 15) == 15))
                    {
                        tileValue(SimulationTarget.x, SimulationTarget.y) = River;
                    }
                    else
                    {
                        tileValue(SimulationTarget.x, SimulationTarget.y) = Rubble + (rand16() & 3) | BulldozableBit;
                    }
                    return;
                }
            }
        }
    }

    if (!(CurrentTile & BurnableBit)) /* If Bridge */
    {
        RoadCount += 4;
        if (DoBridge())
        {
            return;
        }
    }

    int trafficDensity{};

    if (CurrentTileMasked < TrafficLightBase)
    {
        trafficDensity = 0;
    }
    else if (CurrentTileMasked < TrafficHeavyBase)
    {
        trafficDensity = 1;
    }
    else
    {
        RoadCount++;
        trafficDensity = 2;
    }

    // Set Traffic Density
    int Density = std::clamp(TrafficDensityMap.value(SimulationTarget.skewInverseBy({ 2, 2 })) / 64, 0, 2);

    // tden 0..2
    if (trafficDensity != Density)
    {
        int z = ((CurrentTileMasked - BridgeBase) & 15) + DensityTable[Density];
        
        z += CurrentTile & (UpperMask - AnimatedBit);
        
        if (Density)
        {
            z += AnimatedBit;
        }

        tileValue(SimulationTarget.x, SimulationTarget.y) = z;
    }
}


/* comefrom: updateSpecialZones spawnHospital */
void RepairZone(int ZCent, int zsize)
{
  int cnt;
  int x, y, ThCh;

  zsize--;
  cnt = 0;
  for (y = -1; y < zsize; y++)
    for (x = -1; x < zsize; x++) {
      int xx = SimulationTarget.x + x;
      int yy = SimulationTarget.y + y;
      cnt++;
      if (coordinatesValid({ xx, yy })) {
	ThCh = tileValue(xx, yy);
	if (ThCh & ZonedBit) continue;
	if (ThCh & AnimatedBit) continue;
	ThCh = ThCh & LowerMask;
	if ((ThCh < Rubble) || (ThCh >= BridgeBase)) {
	  tileValue(xx, yy) = ZCent - 3 - zsize + cnt + ConductiveBit + BurnableBit;
	}
      }
    }
}


/* comefrom: updateSpecialZones */
void DrawStadium(int z)
{
    z = z - 5;
    for (int y = (SimulationTarget.y - 1); y < (SimulationTarget.y + 3); y++)
    {
        for (int x = (SimulationTarget.x - 1); x < (SimulationTarget.x + 3); x++)
        {
            tileValue(x, y) = (z++) | BurnableConductiveBits;
        }
    }
 
    tileValue(SimulationTarget.x, SimulationTarget.y) |= ZonedBit | PowerBit;
}


void CoalSmoke(int mx, int my)
{
    static int SmTb[4] = { CoalPowerSmoke1, CoalPowerSmoke2, CoalPowerSmoke3, CoalPowerSmoke4 };
    static int dx[4] = { 1,  2,  1,  2 };
    static int dy[4] = { -1, -1,  0,  0 };
    int x;

    for (x = 0; x < 4; x++)
    {
        tileValue(mx + dx[x], my + dy[x]) = SmTb[x] | AnimatedBit | ConductiveBit | PowerBit | BurnableBit;
    }
}


/*
 * fixme: Break this into smaller chunks
 */
void updateSpecialZones(bool powered, const CityProperties& properties)
{
    static int MltdwnTab[3] = { 30000, 20000, 10000 };  /* simadj */
    int z;

    switch (CurrentTileMasked)
    {
    case PowerPlant:
        CoalPowerPlantCount++;
        if (!(CityTime & 7)) /* post */
        {
            RepairZone(PowerPlant, 4);
        }
        pushPowerStack(SimulationTarget);
        CoalSmoke(SimulationTarget.x, SimulationTarget.y);
        return;

    case NuclearPower:
        if (gameplayOptions().disastersEnabled && !randomRange(0, MltdwnTab[properties.GameLevel()]))
        {
            DoMeltdown(SimulationTarget.x, SimulationTarget.y);
            return;
        }
        NuclearPowerPlantCount++;
        if (!(CityTime & 7)) /* post */
        {
            RepairZone(NuclearPower, 4);
        }
        pushPowerStack(SimulationTarget);
        return;

    case FireStation:
        FireStationCount++;
        if (!(CityTime & 7)) /* post */
        {
            RepairZone(FireStation, 3);
        }

        if (powered) /* if powered get effect  */
        {
            z = FireEffect;
        }
        else /* from the funding ratio  */
        {
            z = FireEffect / 2;
        }

        if (!roadOnZonePerimeter()) /* post FD's need roads  */
        {
            z = z / 2;
        }

        {
            const auto fstVal = FireStationMap.value({ SimulationTarget.x >> 3, SimulationTarget.y >> 3 });
            FireStationMap.value({ SimulationTarget.x >> 3, SimulationTarget.y >> 3 }) = fstVal + z;
        }
        return;

    case PoliceStation:
        PoliceStationCount++;
        if (!(CityTime & 7))
        {
            RepairZone(PoliceStation, 3); /* post */
        }

        if (powered)
        {
            z = PoliceEffect;
        }
        else
        {
            z = PoliceEffect / 2;
        }

        if (!roadOnZonePerimeter())
        {
            z = z / 2; /* post PD's need roads */
        }

        {
            const auto pstVal = PoliceStationMap.value({ SimulationTarget.x >> 3, SimulationTarget.y >> 3 });
            PoliceStationMap.value({ SimulationTarget.x >> 3, SimulationTarget.y >> 3 }) = pstVal + z;
        }
        return;

    case Stadium:
        StadiumCount++;
        if (!(CityTime & 15))
        {
            RepairZone(Stadium, 4);
        }
        if (powered)
        {
            if (!((CityTime + SimulationTarget.x + SimulationTarget.y) & 31)) // post release
            {
                DrawStadium(StatdiumFull);
                tileValue(SimulationTarget.x + 1, SimulationTarget.y) = FootballGame1 + AnimatedBit;
                tileValue(SimulationTarget.x + 1, SimulationTarget.y + 1) = FootballGame2 + AnimatedBit;
            }
        }
        return;

    case StatdiumFull:
        StadiumCount++;
        if (!((CityTime + SimulationTarget.x + SimulationTarget.y) & 7))	/* post release */
        {
            DrawStadium(Stadium);
        }
        return;

    case Airport:
        AirportCount++;
        
        if (!(CityTime & 7))
        {
            RepairZone(Airport, 6);
        }

        if (powered) // post
        { 
            if ((maskedTileValue(SimulationTarget.x + 1, SimulationTarget.y - 1)) == Radar)
            {
                tileValue(SimulationTarget.x + 1, SimulationTarget.y - 1) = Radar + AnimatedBit + ConductiveBit + BurnableBit;
            }
        }
        else
        {
            tileValue(SimulationTarget.x + 1, SimulationTarget.y - 1) = Radar + ConductiveBit + BurnableBit;
        }

        if (powered)
        {
            DoAirport();
        }
        return;

    case Port:
        SeaPortCount++;
        if ((CityTime & 15) == 0)
        {
            RepairZone(Port, 4);
        }

        SimSprite* shipSprite = getSprite(SimSprite::Type::Ship);
        if ((powered && shipSprite == nullptr) || (shipSprite != nullptr && !shipSprite->active))
        {
            generateShip();
        }
        return;
    }
}


/* comefrom: Simulate DoSimInit */
void MapScan(int x1, int x2, const CityProperties& properties)
{
    for (int x = x1; x < x2; x++)
    {
        for (int y = 0; y < SimHeight; y++)
        {
            CurrentTile = tileValue(x, y);
            if (CurrentTile != 0)
            {
                CurrentTileMasked = maskedTileValue(CurrentTile);

                if (CurrentTileMasked >= Flood)
                {
                    SimulationTarget = { x, y };

                    if (CurrentTileMasked < BridgeBase)
                    {
                        if (CurrentTileMasked >= FireBase)
                        {
                            BurningTileCount++;
                            if (!(rand16() & 3)) // 1 in 4 times
                            {
                                DoFire();
                            }
                            continue;
                        }
                        if (CurrentTileMasked < RadiationTile)
                        {
                            DoFlood();
                        }
                        else
                        {
                            DoRadTile();
                        }
                        continue;
                    }

                    if (CurrentTile & ConductiveBit)
                    {
                        setZonePower({ x, y });
                    }

                    if ((CurrentTileMasked >= BridgeBase) && (CurrentTileMasked < PowerBase))
                    {
                        DoRoad();
                        continue;
                    }

                    if (CurrentTile & ZonedBit) // process Zones
                    {
                        updateZone({ x, y }, properties);
                        continue;
                    }

                    if ((CurrentTileMasked >= RailBase) && (CurrentTileMasked < ResidentialBase))
                    {
                        DoRail({ x, y });
                        continue;
                    }
                    if ((CurrentTileMasked >= ExplosionTinySome) && (CurrentTileMasked <= ExplosionTinyLast)) // clear AniRubble
                    {
                        tileValue(x, y) = Rubble + (rand16() & 3) + BulldozableBit;
                    }
                }
            }
        }
    }
}


void SetValves(const CityProperties& properties, const Budget& budget)
{
    static int TaxTable[21] = {
      200, 150, 120, 100, 80, 50, 30, 0, -10, -40, -100,
      -150, -200, -250, -300, -350, -400, -450, -500, -550, -600 };

    float Employment, Migration, Births, LaborBase, IntMarket;
    float Rratio, Cratio, Iratio, temp;
    float NormResPop, PjResPop, PjComPop, PjIndPop;

    MiscHistory[1] = static_cast<int>(EMarket);
    MiscHistory[2] = ResidentialPopulationCount;
    MiscHistory[3] = CommercialPopulationCount;
    MiscHistory[4] = IndustrialPopulationCount;
    MiscHistory[5] = rci.residentialDemand();
    MiscHistory[6] = rci.commercialDemand();
    MiscHistory[7] = rci.industrialDemand();
    MiscHistory[10] = CrimeRamp;
    MiscHistory[11] = PolluteRamp;
    MiscHistory[12] = LVAverage;
    MiscHistory[13] = CrimeAverage;
    MiscHistory[14] = PolluteAverage;
    MiscHistory[15] = properties.GameLevel();
    MiscHistory[16] = static_cast<int>(cityClass());
    MiscHistory[17] = cityScore();

    NormResPop = static_cast<float>(ResidentialPopulationCount / 8);
    PreviousPopulationTotal = PopulationTotal;
    PopulationTotal = static_cast<int>(NormResPop) + CommercialPopulationCount + IndustrialPopulationCount;

    if (NormResPop) Employment = ((CommercialPopulationHistory[1] + IndustrialPopulationHistory[1]) / NormResPop);
    else Employment = 1;

    Migration = NormResPop * (Employment - 1);
    Births = NormResPop * 0.02f; 			/* Birth Rate  */
    PjResPop = NormResPop + Migration + Births;	/* Projected Res.Pop  */

    if (float result = static_cast<float>(CommercialPopulationHistory[1] + IndustrialPopulationHistory[1]))
    {
        LaborBase = (ResidentialPopulationHistory[1] / result);
    }
    else
    {
        LaborBase = 1.0f;
    }

    if (LaborBase > 1.3f)
    {
        LaborBase = 1.3f;
    }
    if (LaborBase < 0.0f)
    {
        LaborBase = 0.0f;  /* LB > 1 - .1  */
    }

    // Point of this? It adds this all up then just ignores the result?
    for (int z = 0; z < 2; z++)
    {
        temp = static_cast<float>(ResidentialPopulationHistory[z] + CommercialPopulationHistory[z] + IndustrialPopulationHistory[z]);
    }

    IntMarket = (NormResPop + CommercialPopulationCount + IndustrialPopulationCount) / 3.7f;

    PjComPop = IntMarket * LaborBase;

    temp = 1;
    switch (properties.GameLevel())
    {
    case 0:
        temp = 1.2f;
        break;

    case 1:
        temp = 1.1f;
        break;

    case 2:
        temp = 0.98f;
        break;
    }

    PjIndPop = IndustrialPopulationCount * LaborBase * temp;
    if (PjIndPop < 5)
    {
        PjIndPop = 5;
    }

    if (NormResPop)  /* projected -vs- actual */
    {
        Rratio = (PjResPop / NormResPop);
    }
    else
    {
        Rratio = 1.3f;
    }
    if (CommercialPopulationCount)
    {
        Cratio = (PjComPop / CommercialPopulationCount);
    }
    else
    {
        Cratio = PjComPop;
    }
    if (IndustrialPopulationCount)
    {
        Iratio = (PjIndPop / IndustrialPopulationCount);
    }
    else
    {
        Iratio = PjIndPop;
    }

    if (Rratio > 2) { Rratio = 2; }
    if (Cratio > 2) { Cratio = 2; }
    if (Iratio > 2) { Iratio = 2; }

    int index = std::clamp(budget.TaxRate() + properties.GameLevel(), 0, 20);

    Rratio = ((Rratio - 1) * 600) + TaxTable[index]; /* global tax/Glevel effects */
    Cratio = ((Cratio - 1) * 600) + TaxTable[index];
    Iratio = ((Iratio - 1) * 600) + TaxTable[index];

	rci.adjustResidentialDemand(static_cast<int>(Rratio));
	rci.adjustCommercialDemand(static_cast<int>(Cratio));
	rci.adjustIndustrialDemand(static_cast<int>(Iratio));

    if ((ResCap) && (rci.residentialDemand() > 0)) // Stad, Prt, Airprt
    {
        rci.residentialDemand(0);
    }
    if ((ComCap) && (rci.commercialDemand() > 0))
    {
        rci.commercialDemand(0);
    }
    if ((IndCap) && (rci.industrialDemand() > 0))
    {
        rci.industrialDemand(0);
    }
}


void ClearCensus()
{
    PoweredZoneCount = 0;
    UnpoweredZoneCount = 0;
    BurningTileCount = 0;
    RoadCount = 0;
    RailCount = 0;
    ResidentialPopulationCount = 0;
    CommercialPopulationCount = 0;
    IndustrialPopulationCount = 0;
    ResidentialZoneCount = 0;
    CommercialZoneCount = 0;
    IndustrialZoneCount = 0;
    HospitalCount = 0;
    ChurchCount = 0;
    PoliceStationCount = 0;
    FireStationCount = 0;
    StadiumCount = 0;
    CoalPowerPlantCount = 0;
    NuclearPowerPlantCount = 0;
    SeaPortCount = 0;
    AirportCount = 0;
    resetPowerStack(); // Reset before Mapscan

    FireStationMap.reset();
    PoliceStationMap.reset();
}


void TakeCensus(Budget& budget)
{
    /* put census#s in Historical Graphs and scroll data  */
    std::rotate(ResidentialPopulationHistory.rbegin(), ResidentialPopulationHistory.rbegin() + 1, ResidentialPopulationHistory.rend());
    std::rotate(CommercialPopulationHistory.rbegin(), CommercialPopulationHistory.rbegin() + 1, CommercialPopulationHistory.rend());
    std::rotate(IndustrialPopulationHistory.rbegin(), IndustrialPopulationHistory.rbegin() + 1, IndustrialPopulationHistory.rend());
    std::rotate(CrimeHistory.rbegin(), CrimeHistory.rbegin() + 1, CrimeHistory.rend());
    std::rotate(PollutionHistory.rbegin(), PollutionHistory.rbegin() + 1, PollutionHistory.rend());
    std::rotate(MoneyHis.rbegin(), MoneyHis.rbegin() + 1, MoneyHis.rend());

    ResidentialPopulationHistoryHighest = *std::max_element(ResidentialPopulationHistory.begin(), ResidentialPopulationHistory.end());
    CommercialPopulationHistoryHighest = *std::max_element(CommercialPopulationHistory.begin(), CommercialPopulationHistory.end());
    IndustrialPopulationHistoryHighest = *std::max_element(IndustrialPopulationHistory.begin(), IndustrialPopulationHistory.end());

    ResidentialPopulationHistory[0] = ResidentialPopulationCount / 8; // magic number
    CommercialPopulationHistory[0] = CommercialPopulationCount;
    IndustrialPopulationHistory[0] = IndustrialPopulationCount;

    CrimeRamp += (CrimeAverage - CrimeRamp) / 4; // magic number
    CrimeHistory[0] = CrimeRamp;

    PolluteRamp += (PolluteAverage - PolluteRamp) / 4; // magic number
    PollutionHistory[0] = PolluteRamp;

    MoneyHis[0] = std::clamp((budget.CashFlow() / 20) + 128, 0, 255); // scale to 0..255
    CrimeHistory[0] = std::clamp(CrimeHistory[0], 0, 255);
    PollutionHistory[0] = std::clamp(PollutionHistory[0], 0, 255);

    if (HospitalCount < (ResidentialPopulationCount / 256))
    {
        HospitalBuildCount = 1;
    }
    if (HospitalCount > (ResidentialPopulationCount / 256))
    {
        HospitalBuildCount = -1;
    }
    if (HospitalCount == (ResidentialPopulationCount / 256))
    {
        HospitalBuildCount = 0;
    }

    if (ChurchCount < (ResidentialPopulationCount / 256))
    {
        ChurchBuildCount = 1;
    }
    if (ChurchCount > (ResidentialPopulationCount / 256))
    {
        ChurchBuildCount = -1;
    }
    if (ChurchCount == (ResidentialPopulationCount / 256))
    {
        ChurchBuildCount = 0;
    }
}


// Long Term Graphs
void Take2Census()
{
    std::rotate(ResHis120Years.rbegin(), ResHis120Years.rbegin() + 1, ResHis120Years.rend());
    std::rotate(ComHis120Years.rbegin(), ComHis120Years.rbegin() + 1, ComHis120Years.rend());
    std::rotate(IndHis120Years.rbegin(), IndHis120Years.rbegin() + 1, IndHis120Years.rend());
    std::rotate(CrimeHis120Years.rbegin(), CrimeHis120Years.rbegin() + 1, CrimeHis120Years.rend());
    std::rotate(PollutionHis120Years.rbegin(), PollutionHis120Years.rbegin() + 1, PollutionHis120Years.rend());
    std::rotate(MoneyHis120Years.rbegin(), MoneyHis120Years.rbegin() + 1, MoneyHis120Years.rend());

    ResHis120Years[0] = ResidentialPopulationCount / 8; // magic number
    ComHis120Years[0] = CommercialPopulationCount;
    IndHis120Years[0] = IndustrialPopulationCount;

    CrimeHis120Years[0] = CrimeHistory[0];
    PollutionHis120Years[0] = PollutionHistory[0];
    MoneyHis120Years[0] = MoneyHis[0];
}


void CollectTax(const CityProperties& properties, Budget& budget)
{
    static float RLevels[3] = { 0.7f, 0.9f, 1.2f };
    static float FLevels[3] = { 1.4f, 1.2f, 0.8f };

    // XXX: do something with z
    //int z = AvCityTax / 48;  // post
    AvCityTax = 0;
    
    budget.PoliceFundsNeeded(PoliceStationCount * 100);
    budget.FireFundsNeeded(FireStationCount * 100);
    budget.RoadFundsNeeded(static_cast<int>((RoadCount + (RailCount * 2)) * RLevels[properties.GameLevel()]));

    budget.TaxIncome(static_cast<int>(((static_cast<float>(PopulationTotal) * LVAverage) / 120.0f) * budget.TaxRate() * FLevels[properties.GameLevel()])); //yuck

    if (PopulationTotal) // if there are people to tax
    {
        budget.update();
    }
    else
    {
        RoadEffect = 32;
        PoliceEffect = 1000;
        FireEffect = 1000;
    }
}


// tends to empty RateOfGrowthMap
// ROG == Rate Of Growth
void DecROGMem()
{
    for (int x = 0; x < EighthWorldWidth; x++)
    {
        for (int y = 0; y < EighthWorldHeight; y++)
        {
            int z = RateOfGrowthMap.value({ x, y });
            if (z == 0)
            {
                continue;
            }
            if (z > 0)
            {
                const auto rogVal = RateOfGrowthMap.value({ x, y });
                RateOfGrowthMap.value({ x, y }) = rogVal - 1;
                if (z > 200) // prevent overflow
                {
                    RateOfGrowthMap.value({ x, y }) = std::min(z, 200);
                }
                continue;
            }
            if (z < 0)
            {
                const auto rogVal = RateOfGrowthMap.value({ x, y });
                RateOfGrowthMap.value({ x, y }) = rogVal + 1;
                if (z < -200)
                {
                    RateOfGrowthMap.value({ x, y }) = -200;
                }
            }
        }
    }
}


/* comefrom: InitSimMemory SimLoadInit */
void SetCommonInits()
{
    EvalInit();
    RoadEffect = 32;
    PoliceEffect = 1000;
    FireEffect = 1000;
}


void InitSimMemory()
{
    SetCommonInits();

    ResidentialPopulationHistory.fill(0);
    CommercialPopulationHistory.fill(0);
    IndustrialPopulationHistory.fill(0);
    MoneyHis.fill(128); // magic number
    CrimeHistory.fill(0);
    PollutionHistory.fill(0);

    CrimeRamp = 0;
    PolluteRamp = 0;
    PopulationTotal = 0;
	rci.residentialDemand(0);
	rci.commercialDemand(0);
	rci.industrialDemand(0);
    ResCap = 0;
    ComCap = 0;
    IndCap = 0;

    EMarket = 6.0;
    DisasterEvent = 0;
    ScoreType = 0;

    resetPowerStack();
    powerScan();

    InitSimLoad = 0;
}


void DoNilPower()
{
    for (int x = 0; x < SimWidth; x++)
    {
        for (int y = 0; y < SimHeight; y++)
        {
            int z = tileValue(x, y);
            if (z & ZonedBit)
            {
                SimulationTarget = { x, y };
                setZonePower({ x, y });
            }
        }
    }
}


/* tends to empty TrafficDensityMap   */
void DecTrafficMem()
{   
    for (int x = 0; x < HalfWorldWidth; x++)
    {
        for (int y = 0; y < HalfWorldHeight; y++)
        {
            int z = TrafficDensityMap.value({ x, y });
            if (z != 0)
            {
                if (z > 24)
                {
                    if (z > 200)
                    {
                        TrafficDensityMap.value({ x, y }) = z - 34;
                    }
                    else
                    {
                        TrafficDensityMap.value({ x, y }) = z - 24;
                    }
                }
                else TrafficDensityMap.value({ x, y }) = 0;
            }
        }
    }
}


/* comefrom: DoSimInit */
void SimLoadInit(CityProperties& properties)
{
    static int DisTab[9] = { 0, 2, 10, 5, 20, 3, 5, 5, 2 * 48 };
    static int ScoreWaitTab[9] = { 0, 30 * 48, 5 * 48, 5 * 48, 10 * 48,
                     5 * 48, 10 * 48, 5 * 48, 10 * 48 };

    EMarket = (float)MiscHistory[1];
    ResidentialPopulationCount = MiscHistory[2];
    CommercialPopulationCount = MiscHistory[3];
    IndustrialPopulationCount = MiscHistory[4];
    rci.residentialDemand(MiscHistory[5]);
    rci.commercialDemand(MiscHistory[6]);
    rci.industrialDemand(MiscHistory[7]);
    CrimeRamp = MiscHistory[10];
    PolluteRamp = MiscHistory[11];
    LVAverage = MiscHistory[12];
    CrimeAverage = MiscHistory[13];
    PolluteAverage = MiscHistory[14];
    properties.GameLevel(MiscHistory[15]);

    if (CityTime < 0)
    {
        CityTime = 0;
    }

    if (!EMarket)
    {
        EMarket = 4.0;
    }

    SetCommonInits();

    cityClass(static_cast<CityClass>(MiscHistory[16]));
    cityScore(MiscHistory[17]);

    if ((cityClass() > CityClass::Megalopolis) || (cityClass() < CityClass::Village))
    {
        cityClass(CityClass::Village);
    }
    if ((cityScore() > 999) || (cityScore() < 1))
    {
        cityScore(500);
    }

    ResCap = 0;
    ComCap = 0;
    IndCap = 0;

    AvCityTax = (CityTime % 48) * 7; /* post */

    resetPowerMap();

    DoNilPower();

    if (ScenarioID > 8)
    {
        ScenarioID = 0;
    }

    if (ScenarioID)
    {
        DisasterEvent = ScenarioID;
        DisasterWait = DisTab[DisasterEvent];
        ScoreType = DisasterEvent;
        ScoreWait = ScoreWaitTab[DisasterEvent];
    }
    else
    {
        DisasterEvent = 0;
        ScoreType = 0;
    }

    RoadEffect = 32;
    PoliceEffect = 1000; /*post*/
    FireEffect = 1000;
    InitSimLoad = 0;
}


namespace
{
    int PowerScanFrequency[5] = { 1,  2,  4,  5, 6 };
    int PollutionScanFrequency[5] = { 1,  2,  7, 17, 27 };
    int CrimeScanFrequency[5] = { 1,  1,  8, 18, 28 };
    int PopulationDensityScanFrequency[5] = { 1,  1,  9, 19, 29 };
    int FireAnalysisFrequency[5] = { 1,  1, 10, 20, 30 };
};


void Simulate(int mod16, CityProperties& properties, Budget& budget)
{
    int speed = static_cast<int>(simSpeed()); // ew, find a better way to do this

    switch (mod16)
    {
    case 0:
        ++Scycle > 1023 ? Scycle = 0 : Scycle;
        
        if (DoInitialEval)
        {
            DoInitialEval = 0;
            CityEvaluation(budget);
        }
        
        CityTime++;
        AvCityTax += budget.TaxRate(); // post <-- ?
        
        if (!(Scycle % 2))
        {
            SetValves(properties, budget);
        }
        
        ClearCensus();
        break;

    case 1:
        MapScan(0, 1 * SimWidth / 8, properties);
        break;

    case 2:
        MapScan(1 * SimWidth / 8, 2 * SimWidth / 8, properties);
        break;

    case 3:
        MapScan(2 * SimWidth / 8, 3 * SimWidth / 8, properties);
        break;

    case 4:
        MapScan(3 * SimWidth / 8, 4 * SimWidth / 8, properties);
        break;

    case 5:
        MapScan(4 * SimWidth / 8, 5 * SimWidth / 8, properties);
        break;

    case 6:
        MapScan(5 * SimWidth / 8, 6 * SimWidth / 8, properties);
        break;

    case 7:
        MapScan(6 * SimWidth / 8, 7 * SimWidth / 8, properties);
        break;

    case 8:
        MapScan(7 * SimWidth / 8, SimWidth, properties);
        break;

    case 9:
        if (!(CityTime % CensusRate))
        {
            TakeCensus(budget);
        }
        if (!(CityTime % (CensusRate * Constants::MonthCount)))
        {
            Take2Census();
        }

        if (!(CityTime % TaxFrequency))
        {
            CollectTax(properties, budget);
            CityEvaluation(budget);
        }
        break;

    case 10:
        if (!(Scycle % 5))
        {
            DecROGMem();
        }
        DecTrafficMem();
        SendMessages(budget);
        break;

    case 11:
        if (!(Scycle % PowerScanFrequency[speed]))
        {
            powerScan();
        }
        break;

    case 12:
        if (!(Scycle % PollutionScanFrequency[speed]))
        {
            pollutionAndLandValueScan();
        }
        break;

    case 13:
        if (!(Scycle % CrimeScanFrequency[speed]))
        {
            crimeScan();
        }
        break;

    case 14:
        if (!(Scycle % PopulationDensityScanFrequency[speed]))
        {
            scanPopulationDensity();
        }
        break;

    case 15:
        if (!(Scycle % FireAnalysisFrequency[speed]))
        {
            fireAnalysis();
        }
        DoDisasters(properties);
        break;
    }
}


const RCI& currentRCI()
{
    return rci;
}


void SimFrame(CityProperties& properties, Budget& budget)
{
    if (simSpeed() == SimulationSpeed::Paused)
    {
        return;
    }

    if (++Fcycle > 1024)
    {
        Fcycle = 0;
    }

    Simulate(Fcycle % 16, properties, budget);
}


void DoSimInit(CityProperties& properties, Budget& budget)
{
    Fcycle = 0;
    Scycle = 0;

    if (InitSimLoad == 2) 			/* if new city    */
    {
        InitSimMemory();
    }

    if (InitSimLoad == 1)  			/* if city just loaded  */
    {
        SimLoadInit(properties);
    }

    SetValves(properties, budget);
    ClearCensus();
    MapScan(0, SimWidth, properties); /* XXX are you sure ??? */
    powerScan();
    pollutionAndLandValueScan();
    crimeScan();
    scanPopulationDensity();
    fireAnalysis();
    newMap(true);
    PopulationTotal = 1;
    DoInitialEval = 1;
}


void UpdateFundEffects(const Budget& budget)
{
    if (budget.RoadFundsNeeded())
    {
        RoadEffect = (int)(((float)budget.RoadFundsGranted() / (float)budget.RoadFundsNeeded()) * 32.0);
    }
    else
    {
        RoadEffect = 32;
    }

    if (budget.PoliceFundsNeeded())
    {
        PoliceEffect = (int)(((float)budget.PoliceFundsGranted() / (float)budget.PoliceFundsNeeded()) * 1000.0);
    }
    else
    {
        PoliceEffect = 1000;
    }

    if (budget.FireFundsNeeded())
    {
        FireEffect = (int)(((float)budget.FireFundsGranted() / (float)budget.FireFundsNeeded()) * 1000.0);
    }
    else
    {
        FireEffect = 1000;
    }
}


void condemnZone(int Xloc, int Yloc, int ch)
{
    int Xtem, Ytem;
    int XYmax;

    RateOfGrowthMap.value({ Xloc / 8, Yloc / 8 }) -= 20;

    ch = ch & LowerMask;
    if (ch < PortBase)
    {
        XYmax = 2;
    }
    else
    {
        if (ch == Airport)
        {
            XYmax = 5;
        }
        else
        {
            XYmax = 4;
        }
    }

    for (int x = -1; x < XYmax; x++)
    {
        for (int y = -1; y < XYmax; y++)
        {
            Xtem = Xloc + x;
            Ytem = Yloc + y;

            /*  Is this check actually necessary ? It was in the original code
                but since this inspects tiles that are zoned and zones can only
				be placed on valid tiles, my guess is this should never be false.
             */
			if (!coordinatesValid({ Xtem, Ytem }))
            {
                continue;
            }

            if (static_cast<int>(maskedTileValue(Xtem, Ytem)) >= BridgeBase) // post release
            {
                tileValue(Xtem, Ytem) |= BulldozableBit;
            }
        }
    }
}
