// This file is part of Micropolis-SDL2PP
// Micropolis-SDL2PP is based on Micropolis
//
// Copyright © 2022 - 2026 Leeor Dicker
//
// Portions Copyright © 1989-2007 Electronic Arts Inc.
//
// Micropolis-SDL2PP is free software; you can redistribute it and/or modify
// it under the terms of the GNU GPLv3, with additional terms. See the README
// file, included in this distribution, for details.
#include "Month.h"

#include <fstream>
#include <nlohmann/json.hpp>


using json = nlohmann::json;


namespace
{
	bool MonthStringsLoaded{ false };

    std::string MonthStrings[12];

    std::ifstream loadStringsFile()
    {
        std::ifstream stringsFile("res/strings.json");
        if (!stringsFile.is_open())
        {
            throw std::runtime_error("Could not open 'res/strings.json'");
        }
        return stringsFile;
	}


    auto monthsArrayFromJson(const json& data)
    {
        if (!data.contains("strings") || !data["strings"].contains("months"))
        {
            throw std::runtime_error("Strings file missing 'strings.months' structure");
        }
        
        const auto& monthsArray = data["strings"]["months"];
        
        if (!monthsArray.is_array() || monthsArray.size() != 12)
        {
            throw std::runtime_error("Expected 12 months in strings file");
        }

        return monthsArray;
	}


    void loadMonthStrings()
    {
        json data = json::parse(loadStringsFile());

		auto months = monthsArrayFromJson(data);

        for (size_t i = 0; i < 12; ++i)
        {
            MonthStrings[i] = months[i].get<std::string>();
        }

        MonthStringsLoaded = true;
    }
}

const std::string& Month::toString(Month::Enum month)
{
	const int index = static_cast<int>(month);
    if (!MonthStringsLoaded)
    {
        loadMonthStrings();
    }

    return MonthStrings[index];
}
