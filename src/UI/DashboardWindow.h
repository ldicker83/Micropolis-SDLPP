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
#pragma once

#include "WindowBase.h"

#include "../Month.h"
#include "../RCI.h"
#include "../StringRender.h"
#include "../Texture.h"

#include <SDL3/SDL.h>

#include <string>

class DashboardWindow : public WindowBase
{
public:
	DashboardWindow(SDL_Renderer* renderer, const RCI& rci);

public:
	void cityName(const std::string& name);

	void setMessage(const std::string& message);

	void onNewMonth(int);
	void onNewYear(int);

	void draw() override;
	void update() override;

private:
	void drawDate();
	void drawMessage();
	void drawValve();
	
	void onMoved(const Vector<int>&) override;
	void onPositionChanged(const Point<int>& pos) override;

private:
	SDL_Renderer* mRenderer{ nullptr };
	
	Texture mTexture;
	
	const RCI& mRci;

	int mTitleHalfWidth{ 0 };

	SDL_FRect mResidentialValveRect{ 0.0f, 0.0f, 4.0f, 0.0f };
	SDL_FRect mCommercialValveRect{ 0.0f, 0.0f, 4.0f, 0.0f };
	SDL_FRect mIndustrialValveRect{ 0.0f, 0.0f, 4.0f, 0.0f };

	std::string mCityName;
	std::string mMessage;

	Month::Enum mCurrentMonth{ Month::Enum::Jan };
	int mCurrentYear{ 1900 };

	StringRender mStringRenderer;
};
