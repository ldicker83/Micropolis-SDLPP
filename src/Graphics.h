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

#include "Texture.h"

#include "Math/Point.h"
#include "Math/Vector.h"

#include <SDL3/SDL.h>

void turnOffBlending(SDL_Renderer& renderer, const Texture& texture);
void turnOnBlending(SDL_Renderer& renderer, const Texture& texture);
void drawPoint(SDL_Renderer& renderer, const Point<int>& point, const SDL_Color& color);
void drawRect(SDL_Renderer& renderer, const SDL_Rect& rect, const SDL_Color& color);
void initTexture(SDL_Renderer& renderer, Texture& texture, const Vector<int>& dimensions);
