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
#include "Tool.h"

#include "Map.h"

#include "Budget.h"
#include "Connection.h"
#include "GameDataLoader.h"

#include "s_alloc.h"
#include "s_msg.h"
#include "s_sim.h"

#include "w_resrc.h"
#include "w_sound.h"
#include "w_tk.h"
#include "w_update.h"
#include "Util.h"

#include <algorithm>
#include <string>


namespace
{
    const Tool* PendingTool{ nullptr };

    const std::vector<Tool> Tools = GameDataLoader::loadTools();

    Point<int> ToolStart{};
    Point<int> ToolEnd{};

    ZoneStats QueryResult{};
    
    bool canAutoBulldoze(int tileValue)
    {
        // Natural terrain and rubble (rivers, trees, rubble, etc.)
        const bool isNaturalOrRubble = (tileValue >= RiverEdgeFirst) && (tileValue <= RubbleLast);

        // Power lines (excluding power plant base tiles)
        const bool isPowerLine = (tileValue >= (PowerBase + 2)) && (tileValue <= (PowerBase + 12));

        // Active explosions and debris
        const bool isExplosion = (tileValue >= ExplosionTiny) && (tileValue <= (ExplosionTinyLast + 2));

        return isNaturalOrRubble || isPowerLine || isExplosion;
    }
}


const Tool& tool(Tool::Type requested)
{
	return *std::find_if(Tools.begin(), Tools.end(), [requested](const Tool& tool)
        {
            return tool.type == requested;
        });
}


const Tool& pendingTool()
{
    return *PendingTool;
}


void pendingTool(const Tool::Type requestedTool)
{
    PendingTool = &tool(requestedTool);
}


const ZoneStats& queryResult()
{
    return QueryResult;
}


void toolStart(const Point<int>& start)
{
    ToolStart = start;
}


const Point<int>& toolStart()
{
    return ToolStart;
}


void toolEnd(const Point<int>& end)
{
    ToolEnd = end;
}


const Point<int>& toolEnd()
{
    return ToolEnd;
}


ToolResult putDownPark(int mapH, int mapV, Budget& budget)
{
    int tile{};

    if (budget.CanAfford(tool(Tool::Type::Park).cost))
    {
        int value = randomRange(0, 4);

        if (value == 4)
        {
            tile = Fountain | BurnableBit | BulldozableBit | AnimatedBit;
        }
        else
        {
            tile = (value + Woods2) | BurnableBit | BulldozableBit;
        }

        if (tileValue(mapH, mapV) == 0)
        {
            budget.Spend(tool(Tool::Type::Park).cost);
            updateFunds(budget);
            tileValue(mapH, mapV) = tile;
            return ToolResult::Success;
        }

        return ToolResult::InvalidLocation;
    }

    return ToolResult::InsufficientFunds;
}


// Radar?
ToolResult putDownNetwork(int mapH, int mapV, Budget& budget)
{
    int tile = tileValue(mapH, mapV) & LowerMask;

    if ((budget.CurrentFunds() > 0) && canAutoBulldoze(tile))
    {
        tileValue(mapH, mapV) = tile = 0;
        budget.Spend(1);
    }

    if (tile != 0)
    {
        return ToolResult::RequiresBulldozing;
    }

    if (budget.CanAfford(tool(Tool::Type::Network).cost))
    {
        tileValue(mapH, mapV) = TELEBASE | ConductiveBit | BurnableBit | BurnableBit | AnimatedBit;
        budget.Spend(tool(Tool::Type::Network).cost);
        return ToolResult::Success;
    }
    else
    {
        return ToolResult::InsufficientFunds;
    }

}


int checkBigZone(int id, int* deltaHPtr, int* deltaVPtr)
{
    switch (id)
    {
    case PowerPlant: // check coal plant
    case Port: // check sea port
    case NuclearPower: // check nuc plant
    case Stadium: // check stadium
        *deltaHPtr = 0;
        *deltaVPtr = 0;
        return (4);

    case PowerPlant + 1: // check coal plant
    case CoalPowerSmoke3: // check coal plant, smoke
    case CoalPowerSmoke3 + 1: // check coal plant, smoke
    case CoalPowerSmoke3 + 2: // check coal plant, smoke
    case Port + 1: // check sea port
    case NuclearPower + 1: // check nuc plant
    case Stadium + 1: // check stadium
        *deltaHPtr = -1;
        *deltaVPtr = 0;
        return (4);

    case PowerPlant + 4: // check coal plant
    case Port + 4: // check sea port
    case NuclearPower + 4: // check nuc plant
    case Stadium + 4: // check stadium
        *deltaHPtr = 0;
        *deltaVPtr = -1;
        return (4);

    case PowerPlant + 5: // check coal plant
    case Port + 5: // check sea port
    case NuclearPower + 5: // check nuc plant
    case Stadium + 5: // check stadium
        *deltaHPtr = -1;
        *deltaVPtr = -1;
        return (4);

        // check airport
        //*** first row ***
    case Airport:
        *deltaHPtr = 0;
        *deltaVPtr = 0;
        return (6);

    case Airport + 1:
        *deltaHPtr = -1;
        *deltaVPtr = 0;
        return (6);

    case Airport + 2:
        *deltaHPtr = -2;
        *deltaVPtr = 0;
        return (6);

    case Airport + 3:
        *deltaHPtr = -3;
        *deltaVPtr = 0;
        return (6);

        //*** second row ***
    case Airport + 6:
        *deltaHPtr = 0;
        *deltaVPtr = -1;
        return (6);

    case Airport + 7:
        *deltaHPtr = -1;
        *deltaVPtr = -1;
        return (6);

    case Airport + 8:
        *deltaHPtr = -2;
        *deltaVPtr = -1;
        return (6);

    case Airport + 9:
        *deltaHPtr = -3;
        *deltaVPtr = -1;
        return (6);

        //*** third row ***
    case Airport + 12:
        *deltaHPtr = 0;
        *deltaVPtr = -2;
        return (6);

    case Airport + 13:
        *deltaHPtr = -1;
        *deltaVPtr = -2;
        return (6);

    case Airport + 14:
        *deltaHPtr = -2;
        *deltaVPtr = -2;
        return (6);

    case Airport + 15:
        *deltaHPtr = -3;
        *deltaVPtr = -2;
        return (6);

        //*** fourth row ***
    case Airport + 18:
        *deltaHPtr = 0;
        *deltaVPtr = -3;
        return (6);

    case Airport + 19:
        *deltaHPtr = -1;
        *deltaVPtr = -3;
        return (6);

    case Airport + 20:
        *deltaHPtr = -2;
        *deltaVPtr = -3;
        return (6);

    case Airport + 21:
        *deltaHPtr = -3;
        *deltaVPtr = -3;
        return (6);

    default:
        *deltaHPtr = 0;
        *deltaVPtr = 0;
        return (0);
    }
}


int checkSize(int temp)
{
    /* check for the normal com, resl, ind 3x3 zones & the fireDept & PoliceDept */
    if (((temp >= (ResidentialBase - 1)) && (temp <= (PortBase - 1))) ||
        ((temp >= (PowerPlantLast + 1)) && (temp <= (PoliceStation + 4))))
    {
        return (3);
    }
    else if (((temp >= PortBase) && (temp <= PortLast)) ||
        ((temp >= CoalPowerBase) && (temp <= PowerPlantLast)) ||
        ((temp >= StadiumBase) && (temp <= ZoneLast)))
    {
        return (4);
    }
    return (0);
}


void doConnectTile(const int x, const int y, const int w, const int h, Budget& budget)
{
    if (coordinatesValid({ x, y }))
    {
        ConnectTile(x, y, {}, budget);
    }
}


void checkBorder(const int mapX, const int mapY, const int count, Budget& budget)
{
    int xPos{}, yPos{};

    xPos = mapX; yPos = mapY - 1;
    for (int cnt = 0; cnt < count; cnt++)
    {
        /*** this will do the upper bordering row ***/
        doConnectTile(xPos, yPos, SimWidth, SimHeight, budget);
        xPos++;
    }

    xPos = mapX - 1; yPos = mapY;
    for (int cnt = 0; cnt < count; cnt++)
    {
        /*** this will do the left bordering row ***/
        doConnectTile(xPos, yPos, SimWidth, SimHeight, budget);
        yPos++;
    }

    xPos = mapX; yPos = mapY + count;
    for (int cnt = 0; cnt < count; cnt++)
    {
        /*** this will do the bottom bordering row ***/
        doConnectTile(xPos, yPos, SimWidth, SimHeight, budget);
        xPos++;
    }

    xPos = mapX + count; yPos = mapY;
    for (int cnt = 0; cnt < count; cnt++)
    {
        /*** this will do the right bordering row ***/
        doConnectTile(xPos, yPos, SimWidth, SimHeight, budget);
        yPos++;
    }
}


ToolResult checkArea(const Point<int> location, const int base, const Tool& tool, Budget& budget)
{
    if (!pointInRect({ location.x - 1, location.y - 1 }, { 0, 0, SimWidth - tool.size, SimHeight - tool.size }))
    {
        return ToolResult::OutOfBounds;
    }

    bool needsBulldozing{ false };

    int totalCost{ tool.cost };

    int mapY{ location.y - 1 };
    for (int row = 0; row < tool.size; ++row)
    {
        int mapX{ location.x - 1 };

        for (int col = 0; col < tool.size; ++col)
        {
            const unsigned int tileValue{ maskedTileValue(mapX, mapY) };

            if (gameplayOptions().autoBulldoze)
            {
                // if autoDoze is enabled, add up the cost of bulldozed tiles
                if (tileValue != 0)
                {
                    if (canAutoBulldoze(tileValue))
                    {
                        ++totalCost;
                    }
                    else
                    {
                        needsBulldozing = true;
                    }
                }
            }
            else
            {
                // check and see if the tile is clear or not
                if (tileValue != 0)
                {
                    needsBulldozing = true;
                    break;
                }
            }
            ++mapX;
        }
        ++mapY;
    }

    if (needsBulldozing)
    {
        return ToolResult::RequiresBulldozing;
    }

    if (!budget.CanAfford(totalCost))
    {
        return ToolResult::InsufficientFunds;
    }

    budget.Spend(totalCost);
    updateFunds(budget);

    int tileBase = base;
    mapY = location.y - 1;
    for (int row = 0; row < tool.size; ++row)
    {
        int mapX = location.x - 1;

        for (int col = 0; col < tool.size; ++col)
        {
            if (col == 1 && row == 1)
            {
                tileValue(mapX, mapY) = tileBase + BNCNBIT + ZonedBit;
            }
            // special case to get nuclear plant animation working
            else if (tool.type == Tool::Type::Nuclear && col == 1 && row == 2)
            {
                tileValue(mapX, mapY) = tileBase + BNCNBIT + AnimatedBit;
            }
            else
            {
                tileValue(mapX, mapY) = tileBase + BNCNBIT;
            }
            ++mapX;
            ++tileBase;
        }
        ++mapY;
    }

    checkBorder(location.x - 1, location.y - 1, tool.size, budget);
    return ToolResult::Success;
}


/* QUERY */
/* search table for zone status string match */
static int idArray[28] =
{
    Dirt,
    River,
    TreeBase,
    Rubble,
    Flood,
    RadiationTile,
    FireBase,
    BridgeBase,
    PowerBase,
    RailBase,
    ResidentialBase,
    CommercialBase,
    IndustryBase,
    PortBase,
    AirportBase,
    CoalPowerBase,
    FireStationBase,
    PoliceStationBase,
    StadiumBase,
    NuclearPowerBase,
    827, // crossed out lightning bolt
    832, // radar dish first frame
    Fountain,
    INDBASE2,
    FootballGame1,
    VBRDG0,
    952, // radiation icon first frame
    956 // white tile?
};

/*
  0, 2, 21, 44, 
  48, 52, 53, 64,
  208, 224, 240, 423, 
  612, 693, 709, 745,
  761, 770, 779, 811, 
  827, 832, 840, 844,
  932, 948, 952, 956

  Clear, Water, Trees, Rubble, 
  Flood, Radioactive Waste, Fire, Road,
  Power, Rail, Residential, Commercial,
  Industrial, Port, AirPort, Coal Power,
  Fire Department, Police Department, Stadium, Nuclear Power, 
  Draw Bridge, Radar Dish, Fountain, Industrial,
  49er's 38  Bears 3, Draw Bridge, Ur 238
*/


int getDensityStr(int catNo, int mapH, int mapV)
{
    int z;

    switch (catNo)
    {
    case 0:
        z = PopulationDensityMap.value({ mapH >> 1, mapV >> 1 });
        z = z >> 6;
        z = z & 3;
        return (z);

    case 1:
        z = LandValueMap.value({ mapH >> 1, mapV >> 1 });
        if (z < 30) return (4);
        if (z < 80) return (5);
        if (z < 150) return (6);
        return (7);

    case 2:
        z = CrimeMap.value({ mapH >> 1, mapV >> 1 });
        z = z >> 6;
        z = z & 3;
        return (z + 8);

    case 3:
        z = PollutionMap.value({ mapH >> 1, mapV >> 1 });
        if ((z < 64) && (z > 0)) return (13);
        z = z >> 6;
        z = z & 3;
        return (z + 12);

    case 4:
        z = RateOfGrowthMap.value({ mapH >> 3, mapV >> 3 });
        if (z < 0) return (16);
        if (z == 0) return (17);
        if (z > 100) return (19);
        return (18);

    default:
        throw std::runtime_error("");
    }
}


const std::string& queryString(int tileValue)
{
    for (int i = 1; i < 29; ++i)
    {
        if (tileValue < idArray[i])
        {
            const int queryId = std::clamp(i - 1, 0, 28);
            return QueryStatsString(static_cast<QueryStatsId>(queryId));
        }
    }

    return QueryStatsString(QueryStatsId::Padding2);
}


void doZoneStatus(int x, int y)
{
    int tileNum = maskedTileValue(x, y);
    if (tileNum >= CoalPowerSmoke1 && tileNum < FootballGame1)
    {
        tileNum = CoalPowerBase;
    }

    std::array<std::string, 5> statusStr;
    for (int i = 0; i < 5; ++i)
    {
        const int id = getDensityStr(i, x, y);
        statusStr[i] = ZoneStatsString(static_cast<ZoneStatsId>(std::clamp(id, 1, 19)));
    }

    QueryResult = { queryString(tileNum), statusStr[0], statusStr[1], statusStr[2], statusStr[3], statusStr[4] };
}


void putRubble(const int mapX, const int mapY, const int size)
{
    for (int x = mapX - 1; x < mapX + size - 1; x++)
    {
        for (int y = mapY - 1; y < mapY + size - 1; y++)
        {
            if (coordinatesValid({ x, y }))
            {
                int cellValue = maskedTileValue(x, y);
                if ((cellValue != RadiationTile) && (cellValue != 0))
                {
                    tileValue(x, y) = (gameplayOptions().animationEnabled ? (ExplosionTiny + randomRange(0, 2)) : ExplosionTinySome) | AnimatedBit | BulldozableBit;
                }
            }
        }
    }
}


/************************************************************************/
/* TOOLS */


ToolResult query_tool(int x, int y, Budget&)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    doZoneStatus(x, y);
    return ToolResult::Success;
}


ToolResult bulldozer_tool(int x, int y, Budget& budget)
{
    unsigned int currTile, temp;
    int zoneSize, deltaH, deltaV;

    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    currTile = tileValue(x, y);
    temp = currTile & LowerMask;

    ToolResult result = ToolResult::Success;

    if (currTile & ZonedBit)
    { /* zone center bit is set */
        if (!budget.Broke())
        {
            budget.Spend(1);
            switch (checkSize(temp))
            {
            case 3:
                MakeSound("city", "Explosion-High");
                putRubble(x, y, 3);
                break;

            case 4:
                putRubble(x, y, 4);
                MakeSound("city", "Explosion-Low");
                break;

            case 6:
                MakeSound("city", "Explosion-High");
                MakeSound("city", "Explosion-Low");
                putRubble(x, y, 6);
                break;

            default:
                break;
            }
        }
    }
    else if ((zoneSize = checkBigZone(temp, &deltaH, &deltaV)))
    {
        if (!budget.Broke())
        {
            budget.Spend(1);
            switch (zoneSize)
            {
            case 3:
                MakeSound("city", "Explosion-High");
                break;

            case 4:
                MakeSound("city", "Explosion-Low");
                putRubble(x + deltaH, y + deltaV, 4);
                break;

            case 6:
                MakeSound("city", "Explosion-High");
                MakeSound("city", "Explosion-Low");
                putRubble(x + deltaH, y + deltaV, 6);
                break;
            }
        }
    }
    else
    {
        if (temp == River || temp == RiverEdge || temp == RiverChannel)
        {
            if (budget.CanAfford(5)) /// \fixme Magic Number
            {
                result = ConnectTile(x, y, tool(Tool::Type::Bulldoze), budget);
                if (temp != (tileValue(x, y) & LowerMask))
                {
                    budget.Spend(5);
                }
            }
            else
            {
                result = ToolResult::InsufficientFunds;
            }
        }
        else
        {
            result = ConnectTile(x, y, tool(Tool::Type::Bulldoze), budget);
        }
    }
    updateFunds(budget);
    return result;
}


ToolResult road_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    ToolResult result = ConnectTile(x, y, tool(Tool::Type::Road), budget);
    updateFunds(budget);
    return result;
}


ToolResult rail_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    ToolResult result = ConnectTile(x, y, tool(Tool::Type::Rail), budget);
    updateFunds(budget);
    return result;
}


ToolResult wire_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    ToolResult result = ConnectTile(x, y, tool(Tool::Type::Wire), budget);
    updateFunds(budget);
    return result;
}


ToolResult park_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    return putDownPark(x, y, budget);
}


ToolResult residential_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    return checkArea({ x, y }, ResidentialBase, *PendingTool, budget);
}


ToolResult commercial_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    return checkArea({ x, y }, CommercialBase, *PendingTool, budget);
}


ToolResult industrial_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    return checkArea({ x, y }, IndustryBase, *PendingTool, budget);
}


ToolResult police_dept_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    return checkArea({ x, y }, PoliceStationBase, *PendingTool, budget);
}


ToolResult fire_dept_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    return checkArea({ x, y }, FireStationBase, *PendingTool, budget);
}


ToolResult stadium_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    return checkArea({ x, y }, StadiumBase, *PendingTool, budget);
}


ToolResult coal_power_plant_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    return checkArea({ x, y }, CoalPowerBase, *PendingTool, budget);
}


ToolResult nuclear_power_plant_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    return checkArea({ x, y }, NuclearPowerBase, *PendingTool, budget);
}


ToolResult seaport_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    return checkArea({ x, y }, PortBase, *PendingTool, budget);
}


ToolResult airport_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    return checkArea({ x, y }, AirportBase, *PendingTool, budget);
}


ToolResult network_tool(int x, int y, Budget& budget)
{
    if (!coordinatesValid({ x, y }))
    {
        return ToolResult::OutOfBounds;
    }

    return putDownNetwork(x, y, budget);
}


std::map<Tool::Type, ToolResult(*)(int, int, Budget&)> ToolFunctionTable =
{
    { Tool::Type::Residential, &residential_tool },
    { Tool::Type::Commercial, &commercial_tool },
    { Tool::Type::Industrial, &industrial_tool },
    { Tool::Type::Fire, &fire_dept_tool },
    { Tool::Type::Query, &query_tool },
    { Tool::Type::Police, &police_dept_tool },
    { Tool::Type::Wire, &wire_tool },
    { Tool::Type::Bulldoze, &bulldozer_tool },
    { Tool::Type::Rail, &rail_tool },
    { Tool::Type::Road, &road_tool },
    { Tool::Type::Stadium, &stadium_tool },
    { Tool::Type::Park, &park_tool },
    { Tool::Type::Seaport, &seaport_tool },
    { Tool::Type::Coal, &coal_power_plant_tool },
    { Tool::Type::Nuclear, &nuclear_power_plant_tool },
    { Tool::Type::Airport, &airport_tool },
    { Tool::Type::Network, &network_tool },
    { Tool::Type::None, nullptr }
};


/**
 * Performs tool action
 * 
 * Coordinates expected to be in tile coords,
 * not screen coords.
 */
void ToolDown(const Point<int> location, Budget& budget)
{
    if (pendingTool().type == Tool::Type::None)
    {
        return;
    }

    ToolResult result = ToolFunctionTable.at(pendingTool().type)(location.x, location.y, budget);

    if (result == ToolResult::RequiresBulldozing)
    {
        ClearMes();
        SendMes(NotificationId::MustBulldoze);
        MakeSoundOn("edit", "UhUh");
    }
    else if (result == ToolResult::InsufficientFunds)
    {
        ClearMes();
        SendMes(NotificationId::InsufficientFunds);
        MakeSoundOn("edit", "Sorry");
    }
}


int longestAxis(const Vector<int>& vec)
{
    return abs(vec.x) >= abs(vec.y) ? vec.x : vec.y;
}


/**
 * Modifies \c toolVector
 */
void validateDraggableToolVector(Vector<int>& toolVector, Budget& budget)
{
    const int axisLarge = longestAxis(toolVector);
    if (axisLarge == 0) { return; }

    const int step = axisLarge < 0 ? -1 : 1;
    const bool xAxisLarger = std::abs(toolVector.x) > std::abs(toolVector.y);

    const Point<int>& origin = toolStart();
    
    if (CanConnectTile(origin.x, origin.y, pendingTool(), budget) != ToolResult::Success)
    {
        toolVector = {};
        return;
    }

    if (xAxisLarger)
    {
        for (int i = 0; std::abs(i) <= std::abs(toolVector.x); i += step)
        {
            const auto result = CanConnectTile(origin.x + i, origin.y, pendingTool(), budget);
            if (result != ToolResult::Success)
            {
                toolVector = { i - step, 0 };
                return;
            }
        }
    }
    else // ew, find a better way to do this
    {
        for (int i = 0; std::abs(i) <= std::abs(toolVector.y); i += step)
        {
            if (CanConnectTile(origin.x, origin.y + i, pendingTool(), budget) != ToolResult::Success)
            {
                toolVector = { 0, i - step };
                return;
            }
        }
    }
}


void executeDraggableTool(const Vector<int>& toolVector, const Point<int>& tilePointedAt, Budget& budget)
{
    if (toolVector == Vector<int>{ 0, 0 })
    {
        ToolDown(tilePointedAt, budget);
        return;
    }

    const bool xAxisLarger = std::abs(toolVector.x) > std::abs(toolVector.y);
    const int axis = longestAxis(toolVector);
    const int step = axis < 0 ? -1 : 1;

    if (xAxisLarger)
    {
        for (int i = 0; std::abs(i) <= std::abs(toolVector.x); i += step)
        {
            // note: Type cast is temporary
            ConnectTile(toolStart().x + i, toolStart().y, pendingTool(), budget);
        }
    }
    else
    {
        for (int i = 0; std::abs(i) <= std::abs(toolVector.y); i += step)
        {
            // note: Type cast is temporary
            ConnectTile(toolStart().x, toolStart().y + i, pendingTool(), budget);
        }
    }
}
