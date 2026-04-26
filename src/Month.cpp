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
#include "Month.h"

#include "Constants.h"
#include "GameDataLoader.h"

#include <array>
#include <fstream>
#include <nlohmann/json.hpp>


using json = nlohmann::json;


namespace
{
	std::array<std::string, Constants::MonthCount> MonthStrings;
}

const std::string& Month::toString(Month::Enum month)
{
    if (MonthStrings[0].empty())
    {
        MonthStrings = GameDataLoader::loadMonthStrings();
    }

    return MonthStrings[static_cast<int>(month)];
}
