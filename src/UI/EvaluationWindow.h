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

#include "../Evaluation.h"
#include "../Font.h"
#include "../StringRender.h"
#include "../Texture.h"

#include <memory>
#include <SDL3/SDL.h>


class EvaluationWindow : public WindowBase
{
public:
    EvaluationWindow() = delete;
    EvaluationWindow(const EvaluationWindow&) = delete;
    const EvaluationWindow operator=(const EvaluationWindow&) = delete;
    
    EvaluationWindow(SDL_Renderer* renderer);

    void setEvaluation(const Evaluation& evaluation);
    
    void draw() override;
    void update() override {}

private:
    void drawYesNoPanel();
    void drawOpinionPanel();
    void drawStatsPanel();
    void drawScorePanel();
    
private:
    std::unique_ptr<Font> mFont;
    std::unique_ptr<Font> mFontBold;
    std::unique_ptr<Font> mFontSemiBold;

    const int mLineSpacing{ 0 };
    const int mTitleSpacing{ 0 };

    Texture mTexture;
    Texture mTextTexture;

    SDL_Renderer* mRenderer{ nullptr };

    Evaluation mEvaluation;
    StringRender mStringRenderer;
};
