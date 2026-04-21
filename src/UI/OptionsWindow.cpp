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
#include "OptionsWindow.h"

#include "../Util.h"

#include <algorithm>
#include <array>
#include <map>


namespace
{
	constexpr SDL_FRect BgRect{ 0.0f, 0.0f, 256.0f, 256.0f };
	constexpr SDL_FRect CheckedBox{ 261, 13, 14, 12 };

	enum class CheckBox
	{
		AutoBudget,
		AutoBulldoze,
		AutoGoto,
		DisastersEnabled,
		PlayMusic,
		PlaySound
	};

	std::array CheckBoxes =
	{
		std::make_tuple(CheckBox::AutoBudget, SDL_Rect{ 145, 172, 12, 12 }, false),
		std::make_tuple(CheckBox::AutoBulldoze, SDL_Rect{ 13, 172, 12, 12 }, false),
		std::make_tuple(CheckBox::AutoGoto, SDL_Rect{ 13, 188, 12, 12 }, false),
		std::make_tuple(CheckBox::DisastersEnabled, SDL_Rect{ 13, 204, 12, 12 }, false),
		std::make_tuple(CheckBox::PlayMusic, SDL_Rect{ 145, 204, 12, 12 }, false),
		std::make_tuple(CheckBox::PlaySound, SDL_Rect{ 145, 188, 12, 12 }, false)
	};

	
	void setOptionsFromCheckboxValues(GameOptions& options)
	{
		options.autoBudget = std::get<bool>(CheckBoxes[0]);
		options.autoBulldoze = std::get<bool>(CheckBoxes[1]);
		options.autoGoto = std::get<bool>(CheckBoxes[2]);
		options.disastersEnabled = std::get<bool>(CheckBoxes[3]);
		options.musicEnabled = std::get<bool>(CheckBoxes[4]);
		options.soundEnabled = std::get<bool>(CheckBoxes[5]);
	}


	template <typename BaseType>
	SDL_Point pointToSdlPoint(const Point<BaseType>& point)
	{
		return { point.x, point.y };
	}


	template <typename BaseType>
	SDL_Rect adjustedRect(const SDL_Rect& source, const Point<BaseType>& offset)
	{
		return { source.x + offset.x, source.y + offset.y, source.w, source.h };
	}


	void postQuit()
	{
		SDL_Event event{};
		event.type = SDL_EVENT_QUIT;
		SDL_PushEvent(&event);
	}
};


OptionsWindow::OptionsWindow(SDL_Renderer* renderer):
	mTexture(loadTexture(renderer, "images/OptionsWindow.png")),
	mCheckTexture(newTexture(renderer, mTexture.dimensions)),
	mRenderer{ renderer }
{
	size({ static_cast<int>(BgRect.w), static_cast<int>(BgRect.h) });
    closeButtonActive(false);
	anchor();

	SDL_SetTextureBlendMode(mCheckTexture.texture, SDL_BLENDMODE_BLEND);

	initButtons();
}


void OptionsWindow::initButtons()
{
	mButtons.emplace_back(Button{ std::bind(&OptionsWindow::buttonReturnClicked, this), SDL_Rect{61, 31, 142, 20} });
	mButtons.emplace_back(Button{ std::bind(&OptionsWindow::buttonNewClicked, this), SDL_Rect{61, 63, 142, 20} });
	mButtons.emplace_back(Button{ std::bind(&OptionsWindow::buttonOpenClicked, this), SDL_Rect{61, 87, 142, 20} });
	mButtons.emplace_back(Button{ std::bind(&OptionsWindow::buttonSaveClicked, this), SDL_Rect{61, 111, 142, 20} });
	mButtons.emplace_back(Button{ std::bind(&OptionsWindow::buttonQuitClicked, this), SDL_Rect{61, 145, 142, 20} });
	mButtons.emplace_back(Button{ std::bind(&OptionsWindow::buttonAcceptClicked, this), SDL_Rect{61, 224, 142, 20} });
}


void OptionsWindow::optionsChangedConnect(CallbackOptionsChanged callback)
{
	mOptionsChangedCallback = callback;
}


void OptionsWindow::newGameCallbackConnect(CallbackSignal callback)
{
	mNewGameCallback = callback;
}


void OptionsWindow::openGameCallbackConnect(CallbackSignal callback)
{
	mOpenGameCallback = callback;
}


void OptionsWindow::saveGameCallbackConnect(CallbackSignal callback)
{
	mSaveGameCallback = callback;
}


void OptionsWindow::setOptions(const GameOptions& options)
{
	mOptions = options;

	/**
	 * These options are arranged in alphabetical order
	 * both in the options struct and in the array for
	 * options.
	 */
	std::get<bool>(CheckBoxes[0]) = options.autoBudget;
	std::get<bool>(CheckBoxes[1]) = options.autoBulldoze;
	std::get<bool>(CheckBoxes[2]) = options.autoGoto;
	std::get<bool>(CheckBoxes[3]) = options.disastersEnabled;
	std::get<bool>(CheckBoxes[4]) = options.musicEnabled;
	std::get<bool>(CheckBoxes[5]) = options.soundEnabled;

	drawChecks();
}


void OptionsWindow::draw()
{
	const auto rect = fRectFromRect({ area().position.x, area().position.y, area().size.x, area().size.y });
	SDL_RenderTexture(mRenderer, mTexture.texture, &BgRect, &rect);
	SDL_RenderTexture(mRenderer, mCheckTexture.texture, &BgRect, &rect);
}


void OptionsWindow::onMouseDown(const Point<int>& point)
{
	checkCheckboxesForClick(point);
	
	drawChecks();
	setOptionsFromCheckboxValues(mOptions);
	
	checkButtonsForClick(point);
}


void OptionsWindow::onKeyDown(int32_t key)
{
	switch (key)
	{
	case SDLK_ESCAPE:
		hide();
		break;

	case SDLK_RETURN:
		optionsChangedTrigger();
		hide();
		break;

	default:
		break;
	}
}


void OptionsWindow::onShow()
{
	drawChecks();
}


void OptionsWindow::checkCheckboxesForClick(const Point<int>& point)
{
	for (auto& [checkbox, rect, checked] : CheckBoxes)
	{
		const auto clickPoint = pointToSdlPoint(point);
		const auto adjustedButtonRect = adjustedRect(rect, area().startPoint());

		if (SDL_PointInRect(&clickPoint, &adjustedButtonRect))
		{
			checked = !checked;
		}
	}
}


void OptionsWindow::checkButtonsForClick(const Point<int>& point)
{
	for (auto& button : mButtons)
	{
		const auto clickPoint = pointToSdlPoint(point);
		const auto adjustedButtonRect = adjustedRect(button.area(), area().startPoint());
		if (SDL_PointInRect(&clickPoint, &adjustedButtonRect))
		{
			button.click();
		}
	}
}


void OptionsWindow::buttonReturnClicked()
{
	hide();
}


void OptionsWindow::buttonNewClicked()
{
	mNewGameCallback();
	hide();
}


void OptionsWindow::buttonOpenClicked()
{
	mOpenGameCallback();
	hide();
}


void OptionsWindow::buttonSaveClicked()
{
	mSaveGameCallback();
	hide();
}


void OptionsWindow::buttonAcceptClicked()
{
	optionsChangedTrigger();
	hide();
}


void OptionsWindow::buttonQuitClicked()
{
	postQuit();
}


void OptionsWindow::drawChecks()
{
	flushTexture(mRenderer, mCheckTexture);

	for (const auto& [checkbox, rect, checked] : CheckBoxes)
	{
		if (checked)
		{
			const SDL_FRect adjustedRect = { static_cast<float>(rect.x), static_cast<float>(rect.y), CheckedBox.w, CheckedBox.h };
			SDL_RenderTexture(mRenderer, mTexture.texture, &CheckedBox, &adjustedRect);
		}
	}

	SDL_SetRenderTarget(mRenderer, nullptr);
}


void OptionsWindow::optionsChangedTrigger()
{
	mOptionsChangedCallback(mOptions);
}
