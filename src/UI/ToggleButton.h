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

#include "Button.h"


/**
 * ToggleButton class representing a button that can be toggled on or off.
 */
class ToggleButton : public Button
{
public:
	bool toggled() const
	{
		return mToggled;
	}


	void toggled(bool value)
	{
		mToggled = value;
		click();
	}


	void toggle()
	{
		mToggled = !mToggled;
		click();
	}

	void reset()
	{
		mToggled = false;
		click();
	}

private:
	bool mToggled{ false };
};