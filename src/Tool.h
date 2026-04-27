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

#include <map>
#include <string>

class Budget;

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


struct Tool
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
