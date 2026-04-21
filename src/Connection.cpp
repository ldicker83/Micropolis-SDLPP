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

#include "Budget.h"
#include "Connection.h"

#include "Tool.h"

#include "Util.h"


#include "main.h"


namespace
{
    int _RoadTable[16]
    {
      66, 67, 66, 68,
      67, 67, 69, 73,
      66, 71, 66, 72,
      70, 75, 74, 76
    };

    int _RailTable[16]
    {
      226, 227, 226, 228,
      227, 227, 229, 233,
      226, 231, 226, 232,
      230, 235, 234, 236
    };

    int _WireTable[16]
    {
      210, 211, 210, 212,
      211, 211, 213, 217,
      210, 215, 210, 216,
      214, 219, 218, 220
    };

    unsigned int NeutralizeRoad(int tile)
    {
        unsigned int outValue = tile & LowerMask;
        if ((outValue >= 64) && (outValue <= 207))
        {
            outValue = (tile & 0x000F) + 64;
        }

        return outValue;
    }
}


ToolResult _LayDoze(int x, int y, Budget& budget)
{
    if (budget.Broke())
    {
        return ToolResult::InsufficientFunds; // no mas dinero.
    }

    if (!(tileValue(x, y) & BulldozableBit))
    {
        return ToolResult::CannotBulldoze; // Check dozeable bit.
    }

    switch (NeutralizeRoad(tileValue(x, y)))
    {
    case BridgeHorizontal:
    case BridgeVertical:
    case BRWV:
    case BRWH:
    case HBRDG0:
    case HBRDG1:
    case HBRDG2:
    case HBRDG3:
    case VBRDG0:
    case VBRDG1:
    case VBRDG2:
    case VBRDG3:
    case PowerHorizontalWater:
    case PowerVerticalWater:
    case RailWaterHorizontal:
    case RailWaterVertical: // Dozing over water, replace with water.
        tileValue(x, y) = River;
        break;

    default: // Dozing on land, replace with land.  Simple, eh?
        tileValue(x, y) = Dirt;
        break;
    }

    budget.Spend(1); // Costs $1.00...
    return ToolResult::Success;
}


ToolResult _LayRoad(int x, int y, Budget& budget)
{
    int cost = 10;

    if (budget.CurrentFunds() < 10)
    {
        return ToolResult::InsufficientFunds;
    }

    //switch (Map[x][y] & LowerMask)
    switch(maskedTileValue(x, y))
    {
    case Dirt:
        tileValue(x, y) = RoadHorizontal | BulldozableBit | BurnableBit;
        break;

    case River: // Road on Water
    case RiverEdge:
    case RiverChannel: // Check how to build bridges, if possible.
        if (budget.CurrentFunds() < 50)
        {
            return ToolResult::InsufficientFunds;
        }

        cost = 50;

        if (x < (SimWidth - 1))
        {
            const int adjTile = NeutralizeRoad(tileValue(x + 1, y));
            if ((adjTile == RailVerticalRoadHorizontal) || (adjTile == BridgeHorizontal) || ((adjTile >= RoadHorizontal) && (adjTile <= RoadPowerHorizontal)))
            {
                tileValue(x, y) = BridgeHorizontal | BulldozableBit;
                break;
            }
        }

        if (x > 0)
        {
            const int adjTile = NeutralizeRoad(tileValue(x - 1, y));
            if ((adjTile == RailVerticalRoadHorizontal) || (adjTile == BridgeHorizontal) || ((adjTile >= RoadHorizontal) && (adjTile <= RoadIntersection)))
            {
                tileValue(x, y) = BridgeHorizontal | BulldozableBit;
                break;
            }
        }

        if (y < (SimHeight - 1))
        {
            const int adjTile = NeutralizeRoad(tileValue(x, y + 1));
            if ((adjTile == RailHorizontalRoadVertical) || (adjTile == RoadPowerVertical) || ((adjTile >= BridgeVertical) && (adjTile <= RoadIntersection)))
            {
                tileValue(x, y) = BridgeVertical | BulldozableBit;
                break;
            }
        }

        if (y > 0)
        {
            const int adjTile = NeutralizeRoad(tileValue(x, y - 1));
            if ((adjTile == RailHorizontalRoadVertical) || (adjTile == RoadPowerVertical) || ((adjTile >= BridgeVertical) && (adjTile <= RoadIntersection)))
            {
                tileValue(x, y) = BridgeVertical | BulldozableBit;
                break;
            }
        }

        // Can't do road...
        return ToolResult::InvalidOperation;

    case PowerHorizontal: // Road on power
        tileValue(x, y) = RoadPowerVertical | ConductiveBit | BurnableBit | BulldozableBit;
        break;

    case PowerVertical: // Road on power #2
        tileValue(x, y) = RoadPowerHorizontal | ConductiveBit | BurnableBit | BulldozableBit;
        break;

    case RailHorizontal: // Road on rail
        tileValue(x, y) = RailHorizontalRoadVertical | BurnableBit | BulldozableBit;
        break;

    case RailVertical: // Road on rail #2
        tileValue(x, y) = RailVerticalRoadHorizontal | BurnableBit | BulldozableBit;
        break;

    default: // Can't do road
        return ToolResult::InvalidOperation;
    }

    budget.Spend(cost);
    return ToolResult::Success;
}


ToolResult _LayRail(int x, int y, Budget& budget)
{
    int cost = 20;

    if (budget.CurrentFunds() < 20)
    {
        return ToolResult::InsufficientFunds;
    }

    switch (NeutralizeRoad(maskedTileValue(x, y)))
    {
    case Dirt: // Rail on Dirt
        tileValue(x, y) = RailHorizontal | BulldozableBit | BurnableBit;
        break;

    case River: // Rail on Water
    case RiverEdge:
    case RiverChannel: // Check how to build underwater tunnel, if possible.
        if (budget.CurrentFunds() < 100)
        {
            return ToolResult::InsufficientFunds;
        }
        cost = 100;

        if (x < (SimWidth - 1))
        {
            const int adjTile = NeutralizeRoad(tileValue(x + 1, y));
            if ((adjTile == RailHorizontalPowerVertical) || (adjTile == RailBase) || ((adjTile >= RailHorizontal) && (adjTile <= RailHorizontalRoadVertical)))
            {
                tileValue(x, y) = RailWaterHorizontal | BulldozableBit;
                break;
            }
        }

        if (x > 0)
        {
            const int adjTile = NeutralizeRoad(tileValue(x - 1, y));
            if ((adjTile == RailHorizontalPowerVertical) || (adjTile == RailBase) || ((adjTile > RailWaterVertical) && (adjTile < RailVerticalRoadHorizontal)))
            {
                tileValue(x, y) = RailWaterHorizontal | BulldozableBit;
                break;
            }
        }

        if (y < (SimHeight - 1))
        {
            const int adjTile = NeutralizeRoad(tileValue(x, y + 1));
            if ((adjTile == RailVerticalPowerHorizontal) || (adjTile == RailVerticalRoadHorizontal) || ((adjTile > RailWaterHorizontal) && (adjTile < RailHorizontalRoadVertical)))
            {
                tileValue(x, y) = RailWaterVertical | BulldozableBit;
                break;
            }
        }

        if (y > 0)
        {
            const int adjTile = NeutralizeRoad(tileValue(x, y - 1));
            if ((adjTile == RailVerticalPowerHorizontal) || (adjTile == RailVerticalRoadHorizontal) || ((adjTile > RailWaterHorizontal) && (adjTile < RailHorizontalRoadVertical)))
            {
                tileValue(x, y) = RailWaterVertical | BulldozableBit;
                break;
            }
        }

        // Can't do rail...
        return ToolResult::InvalidOperation;

    case PowerHorizontal: // Rail on power
        tileValue(x, y) = RailVerticalPowerHorizontal | ConductiveBit | BurnableBit | BulldozableBit;
        break;

    case PowerVertical: // Rail on power #2 
        tileValue(x, y) = RailHorizontalPowerVertical | ConductiveBit | BurnableBit | BulldozableBit;
        break;

    case RoadHorizontal: // Rail on road
        tileValue(x, y) = RailVerticalRoadHorizontal | BurnableBit | BulldozableBit;
        break;

    case RoadVertical: // Rail on road #2
        tileValue(x, y) = RailHorizontalRoadVertical | BurnableBit | BulldozableBit;
        break;

    default: // Can't do rail
        return ToolResult::InvalidOperation;
    }

    budget.Spend(cost);
    return ToolResult::Success;
}


ToolResult _LayWire(int x, int y, Budget& budget)
{
    int cost = 5;

    if (budget.CurrentFunds() < 5)
    {
        return ToolResult::InsufficientFunds;
    }

    switch (NeutralizeRoad(maskedTileValue(x, y)))
    {
    case Dirt: // Wire on Dirt
        tileValue(x, y) = PowerHorizontal | ConductiveBit | BurnableBit | BulldozableBit;
        break;

    case River: // Wire on Water
    case RiverEdge:
    case RiverChannel: // Check how to lay underwater wire, if possible.
        if (budget.CurrentFunds() < 25)
        {
            return ToolResult::InsufficientFunds;
        }

        cost = 25;

        if (x < (SimWidth - 1))
        {
            int adjTile = tileValue(x + 1, y);
            if (adjTile & ConductiveBit)
            {
                adjTile = NeutralizeRoad(adjTile);
                if ((adjTile != 77) && (adjTile != 221) && (adjTile != 208))
                {
                    tileValue(x, y) = PowerVerticalWater | ConductiveBit | BulldozableBit;
                    break;
                }
            }
        }

        if (x > 0)
        {
            int adjTile = tileValue(x - 1, y);
            if (adjTile & ConductiveBit)
            {
                adjTile = NeutralizeRoad(adjTile);
                if ((adjTile != 77) && (adjTile != 221) && (adjTile != 208))
                {
                    tileValue(x, y) = PowerVerticalWater | ConductiveBit | BulldozableBit;
                    break;
                }
            }
        }

        if (y < (SimHeight - 1))
        {
            int adjTile = tileValue(x, y + 1);
            if (adjTile & ConductiveBit)
            {
                adjTile = NeutralizeRoad(adjTile);
                if ((adjTile != 78) && (adjTile != 222) && (adjTile != 209))
                {
                    tileValue(x, y) = PowerHorizontalWater | ConductiveBit | BulldozableBit;
                    break;
                }
            }
        }

        if (y > 0)
        {
            int adjTile = tileValue(x, y - 1);
            if (adjTile & ConductiveBit)
            {
                adjTile = NeutralizeRoad(adjTile);
                if ((adjTile != 78) && (adjTile != 222) && (adjTile != 209))
                {
                    tileValue(x, y) = PowerHorizontalWater | ConductiveBit | BulldozableBit;
                    break;
                }
            }
        }

        // Can't do wire...
        return ToolResult::InvalidOperation;

    case RoadHorizontal: // Wire on Road
        tileValue(x, y) = RoadPowerHorizontal | ConductiveBit | BurnableBit | BulldozableBit;
        break;

    case RoadVertical: // Wire on Road #2
        tileValue(x, y) = RoadPowerVertical | ConductiveBit | BurnableBit | BulldozableBit;
        break;

    case RailHorizontal: // Wire on rail
        tileValue(x, y) = RailHorizontalPowerVertical | ConductiveBit | BurnableBit | BulldozableBit;
        break;

    case RailVertical: // Wire on rail #2
        tileValue(x, y) = RailVerticalPowerHorizontal | ConductiveBit | BurnableBit | BulldozableBit;
        break;

    default: // Can't do wire
        return ToolResult::InvalidOperation;
    }

    budget.Spend(cost);
    return ToolResult::Success;
}


void _FixSingle(int x, int y)
{
    int Tile = NeutralizeRoad(maskedTileValue(x, y));
    int adjTile = 0;

    // Cleanup Road
    if ((Tile >= 66) && (Tile <= 76))
    {
        if (y > 0)
        {
            Tile = NeutralizeRoad(tileValue(x, y - 1));
            if (((Tile == 237) || ((Tile >= 64) && (Tile <= 78))) && (Tile != 77) && (Tile != 238) && (Tile != 64))
            {
                adjTile |= 0x0001;
            }
        }

        if (x < (SimWidth - 1))
        {
            Tile = NeutralizeRoad(tileValue(x + 1, y));
            if (((Tile == 238) || ((Tile >= 64) && (Tile <= 78))) && (Tile != 78) && (Tile != 237) && (Tile != 65))
            {
                adjTile |= 0x0002;
            }
        }

        if (y < (SimHeight - 1))
        {
            Tile = NeutralizeRoad(tileValue(x, y + 1));
            if (((Tile == 237) || ((Tile >= 64) && (Tile <= 78))) && (Tile != 77) && (Tile != 238) && (Tile != 64))
            {
                adjTile |= 0x0004;
            }
        }

        if (x > 0)
        {
            Tile = NeutralizeRoad(tileValue(x - 1, y));
            if (((Tile == 238) || ((Tile >= 64) && (Tile <= 78))) && (Tile != 78) && (Tile != 237) && (Tile != 65))
            {
                adjTile |= 0x0008;
            }
        }

        tileValue(x, y) = _RoadTable[adjTile] | BulldozableBit | BurnableBit;
        return;
    }

    // Cleanup Rail
    if ((Tile >= 226) && (Tile <= 236))
    {

        if (y > 0)
        {
            Tile = NeutralizeRoad(tileValue(x, y - 1));
            if ((Tile >= 221) && (Tile <= 238) && (Tile != 221) && (Tile != 237) && (Tile != 224))
            {
                adjTile |= 0x0001;
            }
        }

        if (x < (SimWidth - 1))
        {
            Tile = NeutralizeRoad(tileValue(x + 1, y));
            if ((Tile >= 221) && (Tile <= 238) && (Tile != 222) && (Tile != 238) && (Tile != 225))
            {
                adjTile |= 0x0002;
            }
        }

        if (y < (SimHeight - 1))
        {
            Tile = NeutralizeRoad(tileValue(x, y + 1));
            if ((Tile >= 221) && (Tile <= 238) && (Tile != 221) && (Tile != 237) && (Tile != 224))
            {
                adjTile |= 0x0004;
            }
        }

        if (x > 0)
        {
            Tile = NeutralizeRoad(tileValue(x - 1, y));
            if ((Tile >= 221) && (Tile <= 238) && (Tile != 222) && (Tile != 238) && (Tile != 225))
            {
                adjTile |= 0x0008;
            }
        }

        tileValue(x, y) = _RailTable[adjTile] | BulldozableBit | BurnableBit;
        return;
    }

    // Cleanup Wire
    if ((Tile >= 210) && (Tile <= 220))
    {

        if (y > 0)
        {
            Tile = tileValue(x, y - 1);
            if (Tile & ConductiveBit)
            {
                Tile = NeutralizeRoad(Tile);
                if ((Tile != 209) && (Tile != 78) && (Tile != 222))
                {
                    adjTile |= 0x0001;
                }
            }
        }

        if (x < (SimWidth - 1))
        {
            Tile = tileValue(x + 1, y);
            if (Tile & ConductiveBit)
            {
                Tile = NeutralizeRoad(Tile);
                if ((Tile != 208) && (Tile != 77) && (Tile != 221))
                {
                    adjTile |= 0x0002;
                }
            }
        }

        if (y < (SimHeight - 1))
        {
            Tile = tileValue(x, y + 1);
            if (Tile & ConductiveBit)
            {
                Tile = NeutralizeRoad(Tile);
                if ((Tile != 209) && (Tile != 78) && (Tile != 222))
                {
                    adjTile |= 0x0004;
                }
            }
        }

        if (x > 0)
        {
            Tile = tileValue(x - 1, y);
            if (Tile & ConductiveBit)
            {
                Tile = NeutralizeRoad(Tile);
                if ((Tile != 208) && (Tile != 77) && (Tile != 221))
                {
                    adjTile |= 0x0008;
                }
            }
        }

        tileValue(x, y) = _WireTable[adjTile] | BulldozableBit | BurnableBit | ConductiveBit;
        return;
    }
}


void _FixZone(int x, int y)
{
    _FixSingle(x, y);

    if (y > 0)
    {
        _FixSingle(x, y - 1);
    }

    if (x < (SimWidth - 1))
    {
        _FixSingle(x + 1, y);
    }

    if (y < (SimHeight - 1))
    {
        _FixSingle(x, y + 1);
    }

    if (x > 0)
    {
        _FixSingle(x - 1, y);
    }
}


ToolResult CanConnectTile(int x, int y, Tool tool, Budget& budget)
{
    const int cost = toolProperties(tool).cost;

    if (budget.CurrentFunds() < cost)
    {
        return ToolResult::InsufficientFunds;
    }

    if (gameplayOptions().autoBulldoze && (budget.CurrentFunds() > 0) && (tileCanBeBulldozed({x, y})))
    {
        const int tile = NeutralizeRoad(tileValue(x, y));
        // Maybe this should check BULLBIT instead of checking tile values?
        if (((tile >= ExplosionTiny) && (tile <= ExplosionTinyLast)) || ((tile < 64) && (tile != 0)))
        {
            return ToolResult::Success;
        }
    }

    switch (tileValue(x, y))
    {
    case Dirt:
        break;

    case River: // Road on Water
    case RiverEdge:
    case RiverChannel: // Check how to build bridges, if possible.
        return ToolResult::InvalidOperation;

    case RoadHorizontal:
    case RoadVertical:
    case PowerHorizontal:
    case PowerVertical:
    case RailHorizontal:
    case RailVertical:
        break;

    default:
        return ToolResult::CannotBulldoze;
        break;
    }

    return ToolResult::Success;
}


ToolResult ConnectTile(int x, int y, Tool tool, Budget& budget)
{
    int Tile = tileValue(x, y);

    // AutoDoze
    if (tool == Tool::Rail || tool == Tool::Road || tool == Tool::Wire)
    {
        if (gameplayOptions().autoBulldoze && (budget.CurrentFunds() > 0) && (Tile & BulldozableBit))
        {
            Tile = NeutralizeRoad(Tile);
            // Maybe this should check BULLBIT instead of checking tile values?
            if (((Tile >= ExplosionTiny) && (Tile <= ExplosionTinyLast)) || ((Tile < 64) && (Tile != 0)))
            {
                budget.Spend(1);
                tileValue(x, y) = Dirt;
            }
        }
    }

    ToolResult result = ToolResult::Success;
    switch (tool)
    {
    case Tool::None:
        _FixZone(x, y);
        break;

    case Tool::Bulldoze:
        result = _LayDoze(x, y, budget);
        _FixZone(x, y);
        break;

    case Tool::Road:
        result = _LayRoad(x, y, budget);
        _FixZone(x, y);
        break;

    case Tool::Rail:
        result = _LayRail(x, y, budget);
        _FixZone(x, y);
        break;

    case Tool::Wire:
        result = _LayWire(x, y, budget);
        _FixZone(x, y);
        break;

    default:
        throw std::runtime_error("Unsupported flag");
        break;
    }

    return result;
}
