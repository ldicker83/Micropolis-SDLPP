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

#include "Math/Vector.h"

#include <SDL3/SDL.h>

#include <string>

struct Texture final
{
    SDL_Texture* texture{ nullptr };
    SDL_FRect area{};

    Vector<int> dimensions{};
};


Texture loadTexture(SDL_Renderer* renderer, std::string_view filename);
Texture newTexture(SDL_Renderer* renderer, const Vector<int>& dimensions);
Texture buildTexture(SDL_Texture* texture);
void flushTexture(SDL_Renderer* renderer, Texture& texture);
