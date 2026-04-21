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
#include "EvaluationWindow.h"


namespace
{
    const SDL_FRect BgRect{ 0.0f, 0.0f, 503.0f, 256.0f };

    constexpr SDL_Rect YesNoRect{ 12, 50, 234, 70 };
    constexpr SDL_Rect OpinionRect{ 12, 128, 234, 115 };
    constexpr SDL_Rect StatisticsRect{ 256, 50, 234, 193 };

    constexpr Vector<int> ContentPanelPadding{ 2, 2 };

    int DualColumnOffset = 0;

    constexpr auto stringLength = [](std::string a, std::string b)
        {
            return a.length() < b.length();
        };


    struct TextColumnMeta
    {
        StringRender& stringRenderer;
        Font& font;
        const Point<int> startPoint;
        int lineSpacing;
        const std::vector<std::string> columns;
    };

    void drawTextColumn(const TextColumnMeta& textColumnMeta)
    {
        int offset = 0;
        for (auto& columnText : textColumnMeta.columns)
        {
            textColumnMeta.stringRenderer.drawString(textColumnMeta.font, columnText, textColumnMeta.startPoint + Vector<int>{ 0, offset });
            offset += textColumnMeta.font.height();
        }
    }
};


EvaluationWindow::EvaluationWindow(SDL_Renderer* renderer):
    mFont{ new Font("res/raleway-medium.ttf", 13) },
    mFontBold{ new Font("res/raleway-bold.ttf", 13) },
    mFontSemiBold{ new Font("res/Raleway-BoldItalic.ttf", 13) },
    mLineSpacing{ mFont->height() + 2 },
    mTitleSpacing{ mFontBold->height() + 15 },
    mTexture(loadTexture(renderer, "images/EvalWindow.png")),
    mTextTexture(newTexture(renderer, mTexture.dimensions)),
    mRenderer{ renderer },
    mStringRenderer{ renderer }
{
    size({ static_cast<int>(BgRect.w), static_cast<int>(BgRect.h) });

    SDL_SetTextureColorMod(mFont->texture(), 0, 0, 0);
    SDL_SetTextureColorMod(mFontBold->texture(), 0, 0, 0);
    SDL_SetTextureColorMod(mFontSemiBold->texture(), 0, 0, 0);

    SDL_SetTextureBlendMode(mTextTexture.texture, SDL_BLENDMODE_BLEND);
}


void EvaluationWindow::drawYesNoPanel()
{
    Point<int> panelStart =
    {
        YesNoRect.x + ContentPanelPadding.x,
        YesNoRect.y + ContentPanelPadding.y
    };

    mStringRenderer.drawString(*mFontBold, "Is the Mayor doing a good job?", panelStart);

    const TextColumnMeta yesNoLabels
    {
        mStringRenderer,
        * mFontSemiBold,
        panelStart + Vector<int>{ 0, mTitleSpacing },
        mLineSpacing,
        { "Yes", "No" }
    };

    drawTextColumn(yesNoLabels);

    const TextColumnMeta yesNoValues
    {
        mStringRenderer,
        *mFont,
        panelStart + Vector<int>{ DualColumnOffset + 50, mTitleSpacing },
        mLineSpacing,
        { mEvaluation.goodyes, mEvaluation.goodno }
    };

    drawTextColumn(yesNoValues);
}


void EvaluationWindow::drawOpinionPanel()
{
    const Point<int> panelStart =
    {
        OpinionRect.x + ContentPanelPadding.x,
        OpinionRect.y + ContentPanelPadding.y
    };

    mStringRenderer.drawString(*mFontBold, "What are the biggest issues?", panelStart);

    const TextColumnMeta opinionLabels
    {
        mStringRenderer,
        *mFontSemiBold,
        panelStart + Vector<int>{ 0, mTitleSpacing },
        mLineSpacing,
        {
            mEvaluation.problemString[0],
            mEvaluation.problemString[1],
            mEvaluation.problemString[2],
            mEvaluation.problemString[3]
        }
    };

    drawTextColumn(opinionLabels);

    const TextColumnMeta opinionValues
    {
        mStringRenderer,
        *mFont,
        panelStart + Vector<int>{ DualColumnOffset + 50, mTitleSpacing },
        mLineSpacing,
        {
            mEvaluation.problemVote[0],
            mEvaluation.problemVote[1],
            mEvaluation.problemVote[2],
            mEvaluation.problemVote[3]
        }
    };

    drawTextColumn(opinionValues);
}


void EvaluationWindow::drawStatsPanel()
{
    const Point<int> panelStart =
    {
        StatisticsRect.x + ContentPanelPadding.x,
        StatisticsRect.y + ContentPanelPadding.y
    };

    const TextColumnMeta statLabels
    {
        mStringRenderer,
        *mFontSemiBold,
        panelStart,
        mLineSpacing,
        {
            "Population",
            "Net Migration",
            "Assessed Value",
            "City Class",
            "Game Level"
        }
    };

    drawTextColumn(statLabels);

    const auto it = std::max_element(statLabels.columns.begin(), statLabels.columns.end(), stringLength);
    DualColumnOffset = statLabels.font.width(*it) + 45;

    const TextColumnMeta statValues
    {
        mStringRenderer,
        *mFont,
        {
            panelStart.x + DualColumnOffset,
            StatisticsRect.y + ContentPanelPadding.y
        },
        mLineSpacing,
        {
            mEvaluation.pop,
            mEvaluation.delta,
            mEvaluation.assessed_dollars,
            mEvaluation.cityclass,
            mEvaluation.citylevel
        }
    };

    drawTextColumn(statValues);
}


void EvaluationWindow::drawScorePanel()
{
    Point<int> scorePanelStart =
    {
        StatisticsRect.x + ContentPanelPadding.x,
        StatisticsRect.y + ContentPanelPadding.y + mLineSpacing * 6
    };

    mStringRenderer.drawString(*mFontBold, "Overall City Score", scorePanelStart);
    mStringRenderer.drawString(*mFont, "Range 0 - 1000", scorePanelStart + Vector<int>{ 0, mLineSpacing });

    const TextColumnMeta scoreLabels
    {
        mStringRenderer,
        *mFontSemiBold,
        scorePanelStart + Vector<int>{ 0, mLineSpacing * 2 + 10 },
        mLineSpacing,
        { "Current Score", "Annual Change" }
    };

    drawTextColumn(scoreLabels);

    const TextColumnMeta scoreValues
    {
        mStringRenderer,
        *mFont,
        scorePanelStart + Vector<int>{ DualColumnOffset, mLineSpacing * 2 + 10 },
        mLineSpacing,
        { mEvaluation.score, mEvaluation.changed }
    };

    drawTextColumn(scoreValues);

    SDL_SetRenderTarget(mRenderer, nullptr);
}


void EvaluationWindow::setEvaluation(const Evaluation& evaluation)
{
    mEvaluation = evaluation;

    flushTexture(mRenderer, mTextTexture);
    
    drawYesNoPanel();
    drawOpinionPanel();
    drawStatsPanel();
    drawScorePanel();
}


void EvaluationWindow::draw()
{
    const SDL_FRect rect{
        static_cast<float>(area().position.x),
        static_cast<float>(area().position.y),
        static_cast<float>(area().size.x),
        static_cast<float>(area().size.y)
    };
    
    SDL_RenderTexture(mRenderer, mTexture.texture, &BgRect, &rect);
    SDL_RenderTexture(mRenderer, mTextTexture.texture, &BgRect, &rect);
}
