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

#include "w_resrc.h"

#include "Math/Point.h"
#include "Math/Vector.h"

#include <map>
#include <string>

class Budget;

enum class Tool
{
    Residential,
    Commercial,
    Industrial,
    Fire,
    Query,
    Police,
    Wire,
    Bulldoze,
    Rail,
    Road,
    Stadium,
    Park,
    Seaport,
    Coal,
    Nuclear,
    Airport,
    Network,
    None
};


struct ToolProperties
{
    int cost{};
    int size{};
    int offset{};
    bool draggable{ false };
    const std::string name{};
};


enum class ToolResult
{
    Success,
    OutOfBounds,
    InsufficientFunds,
    InvalidLocation,
    InvalidOperation,
    NetworkVotedNo,
    RequiresBulldozing,
    CannotBulldoze
};


struct _Tool
{
    enum class Type
    {
        Residential,
        Commercial,
        Industrial,
        Fire,
        Query,
        Police,
        Wire,
        Bulldoze,
        Rail,
        Road,
        Stadium,
        Park,
        Seaport,
        Coal,
        Nuclear,
        Airport,
        Network,
        None
    };

    Type type{ Type::None };

    int cost{ 0 };
    int size{ 0 };
    int offset{ 0 };

    bool draggable{ false };

    const std::string name{};
};


const _Tool& tool(_Tool::Type);


void ToolDown(const Point<int> location, Budget& budget);
bool tally(int tileValue);

const ZoneStats& queryResult();

const _Tool& pendingTool();
void pendingTool(const _Tool::Type);

void toolStart(const Point<int>&);
const Point<int>& toolStart();

void toolEnd(const Point<int>&);
const Point<int>& toolEnd();

int longestAxis(const Vector<int>&);
void validateDraggableToolVector(Vector<int>&, Budget&);
void executeDraggableTool(const Vector<int>&, const Point<int>&, Budget&);
