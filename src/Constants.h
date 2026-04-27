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

namespace Constants
{
    constexpr int MonthCount = 12;

	namespace Files
	{
		constexpr const char* Strings = "res/strings.json";
		constexpr const char* Tools = "res/tools.json";
	}

	namespace Keys
	{
		constexpr const char* Strings = "strings";
		constexpr const char* Months = "months";
		constexpr const char* Tools = "tools";
		constexpr const char* Type = "type";
		constexpr const char* Cost = "cost";
		constexpr const char* Size = "size";
		constexpr const char* Offset = "offset";
		constexpr const char* Draggable = "draggable";

		namespace ToolTypes
		{
			constexpr const char* Residential = "residential";
			constexpr const char* Commercial = "commercial";
			constexpr const char* Industrial = "industrial";
			constexpr const char* Fire = "fire";
			constexpr const char* Query = "query";
			constexpr const char* Police = "police";
			constexpr const char* Wire = "wire";
			constexpr const char* Bulldoze = "bulldoze";
			constexpr const char* Rail = "rail";
			constexpr const char* Road = "road";
			constexpr const char* Stadium = "stadium";
			constexpr const char* Park = "park";
			constexpr const char* Seaport = "seaport";
			constexpr const char* Coal = "coal";
			constexpr const char* Nuclear = "nuclear";
			constexpr const char* Airport = "airport";
		}
	}

	namespace Errors
	{
		constexpr const char* CouldNotOpenFile = "Could not open '{}'";
		constexpr const char* JsonParseError = "Failed to parse JSON from '{}': {}";
		constexpr const char* JsonDataNotArray = "Invalid JSON type: expected array";
		constexpr const char* StringsFileMissingStrings = "Strings file missing 'strings' structure";
		constexpr const char* StringsFileMissingMonths = "Strings file missing 'strings.months' structure";
		constexpr const char* StringsFileMissingTools = "Strings file missing 'strings.tools' structure";
		constexpr const char* ToolsFileMissingTools = "Tools file missing 'tools' array";
		constexpr const char* ToolKeyNotFound = "Tool key '{}' not found in 'strings.tools'";
		constexpr const char* ToolMissingField = "Tool object missing required field '{}'";
		constexpr const char* ToolTypeUnknown = "Unknown tool type: {}";
		constexpr const char* ExpectedTwelveMonths = "Expected 12 months in strings file";
	}
}
