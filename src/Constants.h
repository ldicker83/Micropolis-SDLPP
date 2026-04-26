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

    namespace FilePaths
    {
        constexpr const char* Strings = "res/strings.json";
    }

    namespace JsonKeys
    {
        constexpr const char* Strings = "strings";
        constexpr const char* Months = "months";
    }

    namespace ErrorMessages
    {
        constexpr const char* StringsFileMissingMonths = "Strings file missing 'strings.months' structure";
        constexpr const char* ExpectedTwelveMonths = "Expected 12 months in strings file";
    }
}
