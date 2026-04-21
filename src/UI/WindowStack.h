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

#include "../Math/Point.h"

#include <list>


class WindowBase;


class WindowStack final
{
public:
	WindowStack() = default;
	~WindowStack() = default;

	void addWindow(WindowBase* window);
	void removeWindow(WindowBase* window);

	WindowBase* front();

	bool windowVisible() const;

	bool pointInWindow(const Point<int>& point) const;

	void updateStack(const Point<int>& point);

	void bringToFront(WindowBase* window);

	void hide();

	void draw();
	void update();

	void injectMouseUp();
	void injectMouseMotion(const Vector<int>&);

private:

	std::list<WindowBase*> mWindowList;
};
