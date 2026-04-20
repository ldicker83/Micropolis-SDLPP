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
#include "w_update.h"

#include "Budget.h"

#include "main.h"

#include "s_msg.h"

#include "Scan.h"

#include "w_sound.h"
#include "w_tk.h"
#include "Util.h"


#include <algorithm>
#include <limits>
#include <string>
#include <vector>


namespace
{
    int LastCityTime{};
    int LastCityYear{};
    int CurrentCityYear{};
    Month::Enum LastCityMonth{};

    bool NewMonth{ false };

    std::vector<IntDelegate> NewMonthCallbacks;
    std::vector<IntDelegate> NewYearCallbacks;
}


bool newMonth()
{
    return NewMonth;
}


int lastCityTime()
{
    return LastCityTime;
}


void lastCityTime(int tick)
{
    LastCityTime = tick;
}


Month::Enum lastCityMonth()
{
    return LastCityMonth;
}


void lastCityMonth(Month::Enum month)
{
    LastCityMonth = month;
}


int lastCityYear()
{
    return LastCityYear;
}


void lastCityYear(int year)
{
    LastCityYear = year;
}


void setYear(int year)
{
    // Must prevent year from going negative, since it screws up the non-floored modulo arithmetic.
    if (year < StartingYear)
    {
        year = StartingYear;
    }

    year = (year - StartingYear) - (CityTime / 48);
    CityTime += year * 48;
    updateDate();
}


int currentYear()
{
    return CurrentCityYear;
}


void registerNewMonthCallback(IntDelegate callback)
{
    NewMonthCallbacks.push_back(callback);
}


void clearNewMonthCallbacks()
{
    NewMonthCallbacks.clear();
}


void registerNewYearCallback(IntDelegate callback)
{
	NewYearCallbacks.push_back(callback);
}


void clearNewYearCallbacks()
{
	NewYearCallbacks.clear();
}


void updateDate()
{
    constexpr auto megaannum = 1000000; // wierd place for this

    LastCityTime = CityTime / 4;

    CurrentCityYear = (CityTime / 48) + StartingYear;
    Month::Enum month = static_cast<Month::Enum>((CityTime % 48) / 4);

    if (CurrentCityYear >= megaannum)
    {
        setYear(StartingYear);
        CurrentCityYear = StartingYear;
        SendMes(NotificationId::BrownoutsReported);
    }

    doMessage();

    NewMonth = false;
    if ((lastCityYear() != CurrentCityYear) || (lastCityMonth() != month))
    {
        LastCityYear = CurrentCityYear;
		LastCityMonth = month;

		NewMonth = true;

		for (const auto& callback : NewMonthCallbacks)
		{
			callback(static_cast<int>(month));
		}

        for (const auto& callback : NewYearCallbacks)
        {
            callback(CurrentCityYear);
		}

		// \fixme   This is inelegant. Find a better way to do this without
		//          having to call back into a global function from here.
		if (month == Month::Enum::Jan && !gameplayOptions().autoBudget && !newMap())
		{
			showBudgetWindow();
		}
    }
}


void UpdateOptionsMenu(int options)
{
  /*
  char buf[256];
  sprintf(buf, "UISetOptions %d %d %d %d %d %d %d %d",
	  (options&1)?1:0, (options&2)?1:0,
	  (options&4)?1:0, (options&8)?1:0,
	  (options&16)?1:0, (options&32)?1:0,
	  (options&64)?1:0, (options&128)?1:0);
  Eval(buf);
  */
}


void updateFunds(Budget& budget)
{
    budget.PreviousFunds(budget.CurrentFunds());
    budget.CurrentFunds(std::clamp(budget.CurrentFunds(), 0, std::numeric_limits<int>::max()));
}
