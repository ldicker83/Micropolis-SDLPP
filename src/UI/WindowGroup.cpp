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
#include "WindowGroup.h"

#include "WindowBase.h"


bool WindowGroup::windowVisible() const
{
    for (const auto* window : mWindows)
    {
        if (window->visible())
        {
            return true;
        }
    }

    return false;
}


void WindowGroup::addWindow(WindowBase* window)
{
    mWindows.push_back(window);
}


void WindowGroup::hide()
{
    for (auto* window : mWindows)
    {
        window->hide();
    }
}


void WindowGroup::draw()
{
    for (auto* window : mWindows)
    {
        if (window->visible())
        {
            window->draw();
        }
    }
}


void WindowGroup::update()
{
    for (auto* window : mWindows)
    {
        if (window->visible())
        {
            window->update();
        }
    }
}
