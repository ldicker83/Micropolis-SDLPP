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

#include <vector>

class WindowBase;

/**
 * Thin wrapper class for convenience of working with groups of
 * Windows.
 */
class WindowGroup
{
public:
	bool windowVisible() const;

	void addWindow(WindowBase*);

	void hide();

	void draw();
	void update();

private:
	std::vector<WindowBase*> mWindows;
};
