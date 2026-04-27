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

#include "Constants.h"

#include <array>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>


using MonthStringArray = std::array<std::string, Constants::MonthCount>;


struct Tool;


class GameDataLoader
{
public:
	static nlohmann::json loadStrings();
	static MonthStringArray loadMonthStrings();
	static std::vector<Tool> loadTools();
};
