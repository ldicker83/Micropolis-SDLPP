// This file is part of Micropolis-SDL2PP
// Micropolis-SDL2PP is based on Micropolis
//
// Copyright © 2022 - 2026 Leeor Dicker
//
// Portions Copyright © 1989-2007 Electronic Arts Inc.
//
// Micropolis-SDL2PP is free software; you can redistribute it and/or modify
// it under the terms of the GNU GPLv3, with additional terms. See the README
// file, included in this distribution, for details.
#include "DashboardWindow.h"


#include "../Colors.h"
#include "../Font.h"
#include "../Util.h"

#include <memory>

namespace
{
    const SDL_FRect BackgroundArtRect{ 0.0f, 0.0f, 750.0f, 82.0f };
    const SDL_FRect RCISrc{ 0.0f, 83.0f, 32.0f, 11.0f };
    constexpr auto RciValveHeight = 20;

	constexpr int WindowMiddleX = 254;
    constexpr int WindowTitleBuffer = 20;

    std::unique_ptr<Font> TitleFont;
	std::unique_ptr<Font> MessageFont;


    void renderTitleBackground(SDL_Renderer* renderer, const SDL_FRect& area, int mTitleHalfWidth)
    {
        const SDL_FRect titleBarRect{
            area.x + static_cast<float>(WindowMiddleX - mTitleHalfWidth - WindowTitleBuffer),
            area.y + 3.0f,
            static_cast<float>((mTitleHalfWidth + WindowTitleBuffer) * 2),
            15.0f
        };

        SDL_SetRenderDrawColor(renderer, 214, 214, 214, 255);
        SDL_RenderFillRect(renderer, &titleBarRect);
    }


    void drawTitle(SDL_Renderer* renderer, StringRender& stringRenderer, const Rectangle<int>& area, int titleHalfWidth, const std::string& cityName)
    {
        const Point<int> titlePosition{
            area.position.x + WindowMiddleX - titleHalfWidth,
            area.position.y + 4
        };

        stringRenderer.drawString(*TitleFont, cityName, titlePosition);
    }


    void drawValveRect(SDL_Renderer* renderer, const SDL_Color c, SDL_FRect& rect)
    {
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
}


DashboardWindow::DashboardWindow(SDL_Renderer* renderer, const RCI& rci) :
	mRenderer{ renderer },
	mTexture(loadTexture(renderer, "images/DashboardWindow.png")),
	mRci{ rci },
    mStringRenderer{ renderer }
{
	size({ 750, 82 });
	alwaysVisible(true);
	show();

    if (!TitleFont)
    {
		TitleFont = std::make_unique<Font>("res/virtue.ttf", 12);
        SDL_SetTextureColorMod(TitleFont->texture(), 0, 0, 0);
    }

    if(!MessageFont)
    {
        MessageFont = std::make_unique<Font>("res/Raleway-Medium.ttf", 10);
        SDL_SetTextureColorMod(MessageFont->texture(), 0, 0, 0);
	}
}


void DashboardWindow::cityName(const std::string& name)
{
	mCityName = name;
	const auto titleWidth = TitleFont->width(mCityName);

    mTitleHalfWidth = TitleFont->width(mCityName) / 2;
}


void DashboardWindow::setMessage(const std::string& message)
{
	mMessage = message;
}


void DashboardWindow::onNewMonth(int monthId)
{
    mCurrentMonth = static_cast<Month::Enum>(monthId);
}


void DashboardWindow::onNewYear(int year)
{
    mCurrentYear = year;
}


void DashboardWindow::draw()
{
	const auto rect = fRectFromRect({ area().position.x, area().position.y, area().size.x, area().size.y });
	SDL_RenderTexture(mRenderer, mTexture.texture, &BackgroundArtRect, &rect);
	
    renderTitleBackground(mRenderer, rect, mTitleHalfWidth);
    
	drawTitle(mRenderer, mStringRenderer, area(), mTitleHalfWidth, mCityName);
    drawDate();
    drawMessage();

    drawValve();
}


void DashboardWindow::update()
{}


void DashboardWindow::drawDate()
{
    const std::string currentDate = Month::toString(mCurrentMonth) + " " + std::to_string(mCurrentYear);
    const Point<int> datePosition{ area().position.x + 54, area().position.y + 27 };
    mStringRenderer.drawString(*MessageFont, currentDate, datePosition);
}


void DashboardWindow::drawMessage()
{
    mStringRenderer.drawString(*MessageFont, mMessage, { area().position.x + 54, area().position.y + 69 - MessageFont->height() });
}


void DashboardWindow::drawValve()
{
    const auto residentialPercent = static_cast<float>(mRci.residentialDemand()) / 2000.0f;
    const auto commercialPercent = static_cast<float>(mRci.commercialDemand()) / 1500.0f;
    const auto industrialPercent = static_cast<float>(mRci.industrialDemand()) / 1500.0f;

    mResidentialValveRect.h = -(RciValveHeight * residentialPercent);
    mCommercialValveRect.h = -(RciValveHeight * commercialPercent);
    mIndustrialValveRect.h = -(RciValveHeight * industrialPercent);

	drawValveRect(mRenderer, Colors::Green, mResidentialValveRect);
    drawValveRect(mRenderer, Colors::MediumBlue, mCommercialValveRect);
    drawValveRect(mRenderer, Colors::Gold, mIndustrialValveRect);

	const SDL_FPoint rciSrcPoint{ area().position.x + 9.0f + 4.0f, area().position.y + 44.0f };
    const SDL_FRect rciDst{ rciSrcPoint.x, rciSrcPoint.y, 32.0f, 11.0f };
    SDL_RenderTexture(mRenderer, mTexture.texture, &RCISrc, &rciDst);
}


void DashboardWindow::onMoved(const Vector<int>&)
{
    onPositionChanged(area().position);
}


void DashboardWindow::onPositionChanged(const Point<int>& pos)
{
    mResidentialValveRect.x = static_cast<float>(pos.x + 18);
    mResidentialValveRect.y = static_cast<float>(pos.y + 10 + 39);
	mCommercialValveRect.x = static_cast<float>(pos.x + 27);
	mCommercialValveRect.y = static_cast<float>(pos.y + 10 + 39);
	mIndustrialValveRect.x = static_cast<float>(pos.x + 36);
	mIndustrialValveRect.y = static_cast<float>(pos.y + 10 + 39);
}
