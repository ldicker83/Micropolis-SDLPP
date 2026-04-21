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
#include "Zone.h"

#include "s_alloc.h"
#include "s_sim.h"

#include "Util.h"

#include "CityProperties.h"
#include "Map.h"
#include "Power.h"
#include "Traffic.h"

#include <algorithm>


#define ASCBIT (AnimatedBit | ConductiveBit | BurnableBit)
#define REGBIT (ConductiveBit | BurnableBit)

const std::array<Vector<int>, 4> AdjacentVector =
{ {
    {  0, -1 },
    {  1,  0 },
    {  0,  1 },
    { -1,  0 },
} };


const std::array<Vector<int>, 9> AdjacentVector8 =
{ {
    { -1, -1 },
    {  0, -1 },
    {  1, -1 },
    { -1,  0 },
    {  0,  0 },
    {  1,  0 },
    { -1,  1 },
    {  0,  1 },
    {  1,  1 }
} };


/*
 * set bit in MapWord depending on powermap
 */
bool setZonePower(const Point<int>& location)
{
    if (testPowerBit(location))
    {
        tileValue(location) |= PowerBit;
        return true;
    }

    tileValue(location) &= ~PowerBit;
    return false;
}


void zonePlop(const int base)
{
    // Check for fire and flooding
    for (int i{}; i < 9; ++i)
    {
        const Point<int> coordinates = SimulationTarget + AdjacentVector8[i];

        if (coordinatesValid(coordinates))
        {
            int tile = maskedTileValue(coordinates);
            if ((tile >= Flood) && (tile < BridgeBase))
            {
                return;
            }
        }
    }

    int tileBase{ base };
    for (int i{}; i < 9; ++i)
    {
        const Point<int> coordinates = SimulationTarget + AdjacentVector8[i];

        if (coordinatesValid(coordinates))
        {
            tileValue(coordinates) = tileBase + BNCNBIT;
        }

        ++tileBase;
    }

    setZonePower(SimulationTarget);
    tileValue(SimulationTarget) |= ZonedBit + BulldozableBit;
}


void plopResidential(int density, int value)
{
    const int base{ (((value * 4) + density) * 9) + ResidentialZoneBase - 4 };
    zonePlop(base);
}


void plopCommercial(int density, int value)
{
    const int base{ (((value * 5) + density) * 9) + CommercialZoneBase - 4 };
    zonePlop(base);
}


void plopIndustrial(int density, int value)
{
    const int base{ (((value * 4) + density) * 9) + (IndustrialZoneBase - 4) };
    zonePlop(base);
}


int residentialZonePopulation(int tile)
{
    const int density{ (((tile - ResidentialZoneBase) / 9) % 4) };
    return ((density * 8) + 16);
}


int commercialZonePopulation(int tile)
{
    const int density{ (((tile - CommercialZoneBase) / 9) % 5) + 1 };
    return (tile == CommercialEmpty) ? 0 : density;
}


int industrialZonePopulation(int tile)
{
    const int density{ (((tile - IndustrialZoneBase) / 9) % 4) + 1 };
    return (tile == IndustryEmpty) ? 0 : density;
}


void spawnHospital()
{
    if (CurrentTileMasked == Hospital)
    {
        HospitalCount++;

        if (!(CityTime % 16))/*post*/
        {
            RepairZone(Hospital, 3);
        }

        if (HospitalBuildCount == -1)
        {
            if (!randomRange(0, 20))
            {
                zonePlop(ResidentialBase);
            }
        }
    }
}


void spawnChurch()
{
    if (CurrentTileMasked == Church)
    {
        ChurchCount++;

        if (!(CityTime & 16))/*post*/
        {
            RepairZone(Church, 3);
        }

        if (ChurchBuildCount == -1)
        {
            if (!randomRange(0, 20))
            {
                zonePlop(ResidentialBase);
            }
        }
    }
}


/**
 * Handles smoke stack animation updates
 * 
 * \note    Animation tiles are not set properly. Commenting out
 *          the lines below help but a proper animation table
 *          lookup will be needed.
 */
void setSmoke(bool ZonePower)
{
    static const std::array<bool, 8> animateTile = { true, false, true, true, false, false, true, true };
    
    static const int AniTabA[8] = { 0,    0,   32,   40,    0,    0,   48,   56 };
    //static const int AniTabB[8] = { 0,    0,   36,   44,    0,    0,   52,   60 };
    static const int AniTabC[8] = { IND1,    0, IND2, IND4,    0,    0, IND6, IND8 };
    //static const int AniTabD[8] = { IND1,    0, IND3, IND5,    0,    0, IND7, IND9 };
    
    if (CurrentTileMasked < IndustrialZoneBase)
    {
        return;
    }

    int z{ (CurrentTileMasked - IndustrialZoneBase) / 8 };
    z = z % 8;

    if (animateTile[z])
    {
        const Point<int> location{ SimulationTarget + AdjacentVector8[z] };
        if (coordinatesValid(location))
        {
            if (ZonePower)
            {
                if ((maskedTileValue(location)) == AniTabC[z])
                {
                    tileValue(location) = ASCBIT | (SmokeBase + AniTabA[z]);
                    //tileValue(location) = ASCBIT | (SMOKEBASE + AniTabB[z]);
                }
            }
            else
            {
                if ((maskedTileValue(location)) > static_cast<unsigned int>(AniTabC[z]))
                {
                    tileValue(location) = REGBIT | AniTabC[z];
                    //tileValue(location) = REGBIT | AniTabD[z];
                }
            }
        }
    }
}


void makeHospital()
{
    if (HospitalBuildCount > 0)
    {
        zonePlop(Hospital - 4);
        HospitalBuildCount = 0;
        return;
    }
}


void makeChurch()
{
    if (ChurchBuildCount > 0)
    {
        zonePlop(Church - 4);
        ChurchBuildCount = 0;
        return;
    }
}


int getLandValue()
{
    const auto coord{ SimulationTarget.skewInverseBy({ 2, 2 }) };
    
    int landValue{ LandValueMap.value(coord) - PollutionMap.value(coord) };

    if (landValue < 30) 
    {
        return 0;
    }

    if (landValue < 80) 
    {
        return 1;
    }

    if (landValue < 150) 
    {
        return 2;
    }

    return 3;
}


int evaluateHouseLot(int x, int y)
{
    // test for clear lot
    int tile = maskedTileValue({ x, y });
    if (tile && ((tile < ResidentialBase) || (tile > ResidentialBase + 8)))
    {
        return -1;
    }

    int score{ 1 };
    for (int i{}; i < AdjacentVector.size(); ++i)
    {
        const Point<int> coordinates{ Point<int>{x, y} + AdjacentVector[i] };

        // look for road
        if (coordinatesValid(coordinates) && tile && (tile <= RoadLast))
        {
            score++;
        }
    }

    return score;
}


int evaluateResidential(TrafficResult result)
{
    if (result == TrafficResult::NoTransportNearby)
    {
        return -3000;
    }

    int value{ LandValueMap.value(SimulationTarget.skewInverseBy({ 2, 2 })) };
    value -= PollutionMap.value(SimulationTarget.skewInverseBy({ 2, 2 }));

    value = std::clamp(value * 32, 0, 6000);

    return value - 3000;
}


int evaluateCommercial(TrafficResult result)
{
    if (result == TrafficResult::NoTransportNearby)
    {
        return -3000;
    }

    return ComRate.value(SimulationTarget.skewInverseBy({ 8, 8 }));
}


int evaluateIndustrial(TrafficResult result)
{
    if (result == TrafficResult::NoTransportNearby)
    {
        return -1000;
    }

    return 0;
}


void buildHouse(int value)
{
    static const std::array<Vector<int>, 9> searchVector =
    { {
        {  0,  0 }, // Skip center tile
        { -1, -1 },
        {  0, -1 },
        {  1, -1 },
        { -1,  0 },
        {  1,  0 },
        { -1,  1 },
        {  0,  1 },
        {  1,  1 }
    } };

    int bestLocationOffset{};
    int highestScore{};
    for (int i{ 1 }; i < 9; ++i)
    {
        const Point<int> location = SimulationTarget + searchVector[i];
        if (coordinatesValid(location))
        {
            const auto score = evaluateHouseLot(location.x, location.y);
            if (score != 0)
            {
                if (score > highestScore)
                {
                    highestScore = score;
                    bestLocationOffset = i;
                }

                if ((score == highestScore) && !(randomRange(0, 8)))
                {
                    bestLocationOffset = i;
                }
            }
        }
    }

    if (bestLocationOffset != 0)
    {
        const Point<int> location = SimulationTarget + searchVector[bestLocationOffset];

        if (coordinatesValid(location))
        {
            tileValue(location) = House + BLBNCNBIT + randomRange(0, 2) + (value * 3);
        }
    }
}


void increaseRateOfGrowth(int amount)
{
    const auto location = SimulationTarget.skewInverseBy({ 8, 8 });
    RateOfGrowthMap.value(location) += (amount * 4);
}


void increaseResidential(int population, int value)
{
    const int pollution{ PollutionMap.value(SimulationTarget.skewInverseBy({ 2, 2 })) };

    if (pollution > 128)
    {
        return;
    }

    if (CurrentTileMasked == ResidentialEmpty)
    {
        if (population < 8)
        {
            buildHouse(value);
            increaseRateOfGrowth(1);
            return;
        }

        if (PopulationDensityMap.value(SimulationTarget.skewInverseBy({ 2, 2 })) > 64)
        {
            plopResidential(0, value);
            increaseRateOfGrowth(8);
            return;
        }

        return;
    }

    if (population < 40)
    {
        plopResidential((population / 8) - 1, value);
        increaseRateOfGrowth(8);
    }
}


void increaseCommercial(int population, int value)
{
    int z{ LandValueMap.value(SimulationTarget.skewInverseBy({ 2, 2 })) };
    z /= 32;

    if (population > z)
    {
        return;
    }

    if (population < 5)
    {
        plopCommercial(population, value);
        increaseRateOfGrowth(8);
    }
}


void increaseIndustry(int population, int value)
{
    if (population < 4)
    {
        plopIndustrial(population, value);
        increaseRateOfGrowth(8);
    }
}


void convertResidentialToHomes(int value)
{
    tileValue(SimulationTarget) = ResidentialEmpty | BLBNCNBIT | ZonedBit;

    for (int x{ SimulationTarget.x - 1 }; x <= SimulationTarget.x + 1; ++x)
    {
        for (int y{ SimulationTarget.y - 1 }; y <= SimulationTarget.y + 1; ++y)
        {
            const Point<int> coordinates{ x, y };
            if (coordinatesValid(coordinates))
            {
                const auto tile = maskedTileValue(coordinates);
                if (tile != ResidentialEmpty)
                {
                    tileValue(coordinates) = LHTHR + value + randomRange(0, 2) + BLBNCNBIT;
                }
            }
        }
    }
}


void clearResidentialZone()
{
    static const std::array<int, 9> zoneTileOffset = { 0, 3, 6, 1, 4, 7, 2, 5, 8 };

    int index{};
    for (int x{ SimulationTarget.x - 1 }; x <= SimulationTarget.x + 1; ++x)
    {
        for (int y{ SimulationTarget.y - 1 }; y <= SimulationTarget.y + 1; ++y)
        {
            const Point<int> coordinates{ x, y };
            if (coordinatesValid(coordinates))
            {
                const auto tile = maskedTileValue(coordinates);
                if ((tile >= LHTHR) && (tile <= HHTHR))
                {
                    tileValue(coordinates) = ResidentialBase + zoneTileOffset[index] + BLBNCNBIT;
                    return;
                }
            }
            index++;
        }
    }
}


void decreaseResidential(int population, int value)
{
    if (population == 0)
    {
        return;
    }

    if (population > 16)
    {
        plopResidential(((population - 24) / 8), value);
        increaseRateOfGrowth(-8);
        return;
    }

    if (population == 16)
    {
        increaseRateOfGrowth(-8);
        convertResidentialToHomes(value);
    }

    if (population < 16)
    {
        increaseRateOfGrowth(-1);
        clearResidentialZone();
    }
}


void decreaseCommercial(int population, int value)
{
    if (population > 1)
    {
        plopCommercial(population - 2, value);
        increaseRateOfGrowth(-8);
        return;
    }

    if (population == 1)
    {
        zonePlop(CommercialBase);
        increaseRateOfGrowth(-8);
    }
}


void decreaseIndustry(int population, int value)
{
    if (population > 1)
    {
        plopIndustrial(population - 2, value);
        increaseRateOfGrowth(-8);
        return;
    }

    if (population == 1)
    {
        zonePlop(IndustryEmpty - 4);
        increaseRateOfGrowth(-8);
    }
}


int housePopulation()
{
    int count{};
    for (int x{ SimulationTarget.x - 1 }; x <= SimulationTarget.x + 1; ++x)
    {
        for (int y{ SimulationTarget.y - 1 }; y <= SimulationTarget.y + 1; ++y)
        {
            if (coordinatesValid({x, y}))
            {
                const auto tile = maskedTileValue({x, y});
                if ((tile >= LHTHR) && (tile <= HHTHR))
                {
                    ++count;
                }
            }
        }
    }

    return count;
}


void updateIndustry(bool zonePowered)
{
    int zscore;

    setSmoke(zonePowered);

    int zonePopulation{ industrialZonePopulation(CurrentTileMasked) };
    IndustrialPopulationCount += zonePopulation;
    IndustrialZoneCount++;

    TrafficResult trafficResult{ TrafficResult::RouteFound };

    if (zonePopulation > randomRange(0, 5))
    {
        trafficResult = makeTraffic(2);
    }

    if (trafficResult == TrafficResult::NoTransportNearby)
    {
        decreaseIndustry(zonePopulation, randomRange(0, 2));
        return;
    }

    if (!(randomRange(0, 8)))
    {
        zscore = currentRCI().industrialDemand() + evaluateIndustrial(trafficResult);

        if (!zonePowered)
        {
            zscore = -500;
        }

        if ((zscore > -350) && (zscore - 26380) > rand16())
        {
            increaseIndustry(zonePopulation, rand16() & 1);
            return;
        }

        if ((zscore < 350) && (zscore + 26380) < rand16())
        {
            decreaseIndustry(zonePopulation, rand16() & 1);
        }
    }
}


void updateCommercial(bool zonePowered)
{
    int zscore, locvalve, value;

    CommercialZoneCount++;

    int tpop = commercialZonePopulation(CurrentTileMasked);

    CommercialPopulationCount += tpop;

    TrafficResult trafficResult{TrafficResult::RouteFound};

    if (tpop > randomRange(0, 5))
    {
        trafficResult = makeTraffic(1);
    }

    if (trafficResult == TrafficResult::NoTransportNearby)
    {
        value = getLandValue();
        decreaseCommercial(tpop, value);
        return;
    }

    if (!(rand16() & 7))
    {
        locvalve = evaluateCommercial(trafficResult);
        zscore = currentRCI().commercialDemand() + locvalve;

        if (!zonePowered)
        {
            zscore = -500;
        }

        if (trafficResult == TrafficResult::RouteFound && (zscore > -350) && zscore - 26380 > rand16())
        {
            value = getLandValue();
            increaseCommercial(tpop, value);
            return;
        }

        if (zscore < 350 && zscore + 26380 < rand16())
        {
            value = getLandValue();
            decreaseCommercial(tpop, value);
        }
    }
}


void updateResidential(const Point<int>& location, bool zonePowered)
{
    int residentialPopulation = 0, value = 0;

    const auto tileValue{maskedTileValue(location)};

    if (tileValue == ResidentialEmpty)
    {
        residentialPopulation = housePopulation();
    }
    else
    {
        residentialPopulation = residentialZonePopulation(tileValue);
    }

    ResidentialZoneCount++;
    ResidentialPopulationCount += residentialPopulation;

    TrafficResult trafficResult{ TrafficResult::RouteFound };
    if (residentialPopulation > randomRange(0, 35))
    {
        trafficResult = makeTraffic(0);
    }

    if (trafficResult == TrafficResult::NoTransportNearby)
    {
        value = getLandValue();
        decreaseResidential(residentialPopulation, value);
        return;
    }

    if ((tileValue == ResidentialEmpty) || (randomRange(0, 8) == 0))
    {
        int locationValue = evaluateResidential(trafficResult);
        int zoneScore = currentRCI().residentialDemand() + locationValue;
        if (!zonePowered)
        {
            zoneScore = -500;
        }

        if (zoneScore > -350 && zoneScore - 26380 > -rand16())
        {
            if ((!residentialPopulation) && (!(randomRange(0, 4))))
            {
                makeHospital();
                makeChurch();
                return;
            }

            value = getLandValue();
            increaseResidential(residentialPopulation, value);

            return;
        }

        if ((zoneScore < 350) && zoneScore + 26380 < rand16())
        {
            value = getLandValue();
            decreaseResidential(residentialPopulation, value);
        }
    }
}


void updateZone(const Point<int>& location, const CityProperties& properties)
{
    bool zonePowered{ setZonePower(location) };	

    zonePowered ? PoweredZoneCount++ : UnpoweredZoneCount++;

    if (CurrentTileMasked > PortBase) 
    {
        updateSpecialZones(zonePowered, properties);
        return;
    }

    if (CurrentTileMasked < Hospital)
    {
        updateResidential(location, zonePowered);
        return;
    }

    if (CurrentTileMasked < CommercialBase)
    {
        spawnHospital();
        spawnChurch();
        return;
    }

    if (CurrentTileMasked < IndustryBase)
    {
        updateCommercial(zonePowered);
        return;
    }

    updateIndustry(zonePowered);
    return;
}
