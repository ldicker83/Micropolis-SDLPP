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

#include <array>

#include <SDL3/SDL.h>

#include "../Delegate.h"
#include "../Texture.h"
#include "../Tool.h"

#include "../Math/Point.h"

#include "WindowBase.h"


class ToolPalette : public WindowBase
{
public:
    ToolPalette() = delete;
    ToolPalette(const ToolPalette&) = delete;
    const ToolPalette& operator=(const ToolPalette&) = delete;

    ~ToolPalette();

    const int NoSelection = -1;
    
    const int NormalState = 0;
    const int PressedState = 1;
    const int DisabledState = 2;

public:
    ToolPalette(SDL_Renderer* renderer);

    Tool::Type tool() const;
    const Texture& toolGost() const;

	void toolChangedCallback(VoidDelegate handler);

    void draw() override;
    void update() override;

    void cancelTool();

private:
    struct ButtonMeta
    {
        SDL_FRect rect{};
        Tool::Type tool{ Tool::Type::None };

        int state{};

        Texture ghost{};
    };

private:
    void initToolbarUv();
    void loadToolGhosts();
    void setToolValues();
    void setButtonState(int buttonIndex, int buttonState);
    void drawBackground();

    void toolIndex(const int toolIndex);
    int toolIndex() const;

    void onMouseDown(const Point<int>& mousePosition) override;

    void onMoved(const Vector<int>&) override;
    void onPositionChanged(const Point<int>& position) override;

    void updateButtonPositions();

    std::array<SDL_FRect, 80> mToolButtonUV{};
    std::array<ButtonMeta, 20> mToolButtons{};

    SDL_Renderer* mRenderer{ nullptr };
    
    Texture mIcons{};
    Texture mBackground{};

	VoidDelegate mToolChangedHandler;

    int mSelectedIndex{ NoSelection };
    Tool::Type mTool{ Tool::Type::None };
};
