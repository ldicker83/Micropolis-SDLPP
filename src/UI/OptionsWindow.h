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

#include "Button.h"

#include "../Font.h"
#include "../GameOptions.h"
#include "../StringRender.h"
#include "../Texture.h"

#include <memory>
#include <type_traits>
#include <vector>


#include <SDL3/SDL.h>


class OptionsWindow : public WindowBase
{
public:
    using CallbackOptionsChanged = std::function<void(const GameOptions&)>;
    using CallbackSignal = std::function<void(void)>;

public:
    OptionsWindow() = delete;
    OptionsWindow(const OptionsWindow&) = delete;
    const OptionsWindow operator=(const OptionsWindow&) = delete;

    OptionsWindow(SDL_Renderer* renderer);

    void optionsChangedConnect(CallbackOptionsChanged);
    void newGameCallbackConnect(CallbackSignal);
    void openGameCallbackConnect(CallbackSignal);
    void saveGameCallbackConnect(CallbackSignal);

    void setOptions(const GameOptions&);

    void draw() override;
    void update() override {}

private:
    void initButtons();

    void onMouseDown(const Point<int>&) override;
    void onKeyDown(int32_t key) override;

    void onShow() override;

    void checkCheckboxesForClick(const Point<int>& point);

    void checkButtonsForClick(const Point<int>& point);

    void buttonReturnClicked();
    void buttonNewClicked();
    void buttonOpenClicked();
    void buttonSaveClicked();
    void buttonAcceptClicked();
    void buttonQuitClicked();

    void drawChecks();

    void optionsChangedTrigger();

private:
    Texture mTexture;
    Texture mCheckTexture;

    CallbackOptionsChanged mOptionsChangedCallback;

    CallbackSignal mNewGameCallback;
    CallbackSignal mOpenGameCallback;
    CallbackSignal mSaveGameCallback;

    std::vector<Button> mButtons;

    GameOptions mOptions;

    SDL_Renderer* mRenderer{ nullptr };
};
