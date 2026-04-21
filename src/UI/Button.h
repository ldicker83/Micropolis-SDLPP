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

#include "../Delegate.h"

#include <SDL3/SDL.h>

/**
 * Button class representing a clickable area of the UI.
 * 
 * The button is defined by a rectangular area and a callback function that is
 * called when the button is clicked. The button also has an optional user-
 * defined value that can be used to store additional information about the
 * button.
 */
class Button
{
public:
    static constexpr SDL_Rect DefaultSize{ 0, 0, 10, 10 };

    using ClickedCallback = VoidDelegate;

public:
    /**
     * Default c'tor
     * 
     * Initializes the button with a default size and an empty callback.
     */
    Button():
        mClicked{ []() {} }, mArea{ DefaultSize }
    {}


    /**
     * Constructor with parameters.
     */
    Button(ClickedCallback clicked, const SDL_Rect& area = DefaultSize, int userValue = 0) :
        mClicked{ clicked }, mArea(area), mUserValue(userValue)
    {}


    /**
     * Set the area of the button.
     */
    void area(const SDL_Rect& area)
    {
        mArea = area;
    }


    /**
     * Get the area of the button.
     */
    const SDL_Rect& area() const
    {
        return mArea;
    }

    /**
	 * Allow easy assignment of floating point values to the button area which is stored as integers.
     * 
     * \note Values will be truncated, so the caller should ensure that they are valid.
     */
    void areaF(const SDL_FRect& areaF)
    {
        area({ static_cast<int>(areaF.x), static_cast<int>(areaF.y), static_cast<int>(areaF.w), static_cast<int>(areaF.h) });
    }

    /**
     * Allow easy retrieval of floating point values from the button area which is stored as integers.
     */
    const SDL_FRect areaF() const
    {
        return { static_cast<float>(mArea.x), static_cast<float>(mArea.y), static_cast<float>(mArea.w), static_cast<float>(mArea.h) };
	}

    /**
     * Click the button, triggering the assigned callback.
     */
    void click() const
    {
        mClicked();
    }

    /**
     * Set the user-defined value associated with the button.
     */
    void userValue(int value)
    {
        mUserValue = value;
	}

    /**
     * Get the user-defined value associated with the button.
     */
    int userValue() const
    {
        return mUserValue;
	}

private:
    ClickedCallback mClicked;
    SDL_Rect mArea;

	int mUserValue{ 0 };
};
