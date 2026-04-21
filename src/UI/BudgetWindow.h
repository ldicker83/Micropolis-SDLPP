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

#include <memory>

#include <functional>

#include <SDL3/SDL.h>

#include "WindowBase.h"

#include "../Budget.h"
#include "../Font.h"
#include "../StringRender.h"
#include "../Texture.h"

#include "../Math/Point.h"


class BudgetWindow : public WindowBase
{
public:
	enum class ButtonId
	{
		None,
		TaxRateUp,
		TaxRateDown,
		TransportUp,
		TransportDown,
		PoliceUp,
		PoliceDown,
		FireUp,
		FireDown,
		Accept
	};

	static constexpr auto ButtonStateNormal = 0;
	static constexpr auto ButtonStatePressed = 1;

public:
	BudgetWindow() = delete;
	BudgetWindow(const BudgetWindow&) = delete;
	const BudgetWindow& operator=(const BudgetWindow&) = delete;

	BudgetWindow(SDL_Renderer* renderer, Budget& budget);

	void draw() override;
	void update() override;

private:
	void onMouseDown(const Point<int>& pos) override;
	void onMouseUp() override;

	void handleMouseDown(const ButtonId id);

	void onPositionChanged(const Point<int>& pos) override;

	Budget& mBudget;

	SDL_Renderer* mRenderer{ nullptr };
	StringRender mStringRenderer;

	std::unique_ptr<Font> mFont;

	Texture mTexture{};

	ButtonId mButtonDownId{ ButtonId::None };
};
