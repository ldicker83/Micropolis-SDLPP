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

#include "WindowBase.h"

#include "../Font.h"
#include "../StringRender.h"
#include "../Texture.h"

#include <memory>
#include <SDL3/SDL.h>


struct ZoneStats;


class QueryWindow : public WindowBase
{
public:
    QueryWindow() = delete;
    QueryWindow(const QueryWindow&) = delete;
    const QueryWindow operator=(const QueryWindow&) = delete;

    QueryWindow(SDL_Renderer* renderer);

    void setQueryResult(const ZoneStats& stats);

    void draw() override;
    void update() override {}

private:
    void onMouseDown(const Point<int>&) override;

private:
    std::unique_ptr<Font> mFont;
    std::unique_ptr<Font> mFontBold;
    std::unique_ptr<Font> mFontBoldItalic;

    Texture mTexture;
    Texture mTextTexture;

    SDL_Renderer* mRenderer{ nullptr };

    StringRender mStringRenderer;
};
