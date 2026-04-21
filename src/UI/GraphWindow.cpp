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
#include "GraphWindow.h"

#include "../Colors.h"
#include "../Graphics.h"

#include <array>
#include <map>

namespace
{
	const SDL_FRect ButtonUp{ 285.0f, 0.0f, 36.0f, 36.0f };
	const SDL_FRect ButtonDown{ 326.0f, 0.0f, 36.0f, 36.0f };
	const SDL_FRect Bg{ 0.0f, 0.0f, 264.0f, 287.0f };

	const SDL_FRect GraphLayout{ 10.0f, 71.0f, 242.0f, 202.0f };
	SDL_FRect GraphPosition = GraphLayout;

	enum class ButtonId
	{
		Residential,
		Commercial,
		Industrial,
		Pollution,
		Crime,
		Money
	};


	std::map<ButtonId, Graph> HistoryGraphTable
	{
		{ ButtonId::Residential, { ResidentialPopulationHistory, "Residential", Colors::LightGreen, {} } },
		{ ButtonId::Commercial, { CommercialPopulationHistory, "Commercial", Colors::DarkBlue, {} } },
		{ ButtonId::Industrial, { IndustrialPopulationHistory, "Industrial", Colors::Gold, {} } },
		{ ButtonId::Money, { MoneyHis, "Cash Flow", Colors::Turquoise, {} } },
		{ ButtonId::Crime, { CrimeHistory, "Crime", Colors::Red, {} } },
		{ ButtonId::Pollution, { PollutionHistory, "Pollution", Colors::Olive, {} } }
	};


	struct ButtonMeta
	{
		ButtonId id{ ButtonId::Residential };
		SDL_FRect area{ 0.0f };
		SDL_FRect iconPosition{ 0.0f };
		bool toggled{ false };
	};


	const std::map<ButtonId, SDL_FRect> ButtonLayout
	{
		{ ButtonId::Residential, { 13.0f, 31.0f, 36.0f, 36.0f } },
		{ ButtonId::Commercial, { 53.0f, 31.0f, 36.0f, 36.0f } },
		{ ButtonId::Industrial, { 93.0f, 31.0f, 36.0f, 36.0f } },
		{ ButtonId::Pollution, { 133.0f, 31.0f, 36.0f, 36.0f } },
		{ ButtonId::Crime, { 173.0f, 31.0f, 36.0f, 36.0f } },
		{ ButtonId::Money, { 213.0f, 31.0f, 36.0f, 36.0f } },
	};


	std::array<ButtonMeta, 6> Buttons
	{
		ButtonMeta{ ButtonId::Residential, ButtonLayout.at(ButtonId::Residential), { 0 }, true },
		ButtonMeta{ ButtonId::Commercial, ButtonLayout.at(ButtonId::Commercial), { 0 }, true },
		ButtonMeta{ ButtonId::Industrial, ButtonLayout.at(ButtonId::Industrial), { 0 }, true },
		ButtonMeta{ ButtonId::Pollution, ButtonLayout.at(ButtonId::Pollution), { 0 }, true },
		ButtonMeta{ ButtonId::Crime, ButtonLayout.at(ButtonId::Crime), { 0 }, true },
		ButtonMeta{ ButtonId::Money, ButtonLayout.at(ButtonId::Money), { 0 }, true }
	};


	const std::map<ButtonId, SDL_FRect> IconRects
	{
		{ ButtonId::Residential, { 285.0f, 59.0f, 22.0f, 22.0f } },
		{ ButtonId::Commercial, { 285.0f, 82.0f, 22.0f, 22.0f } },
		{ ButtonId::Industrial, { 285.0f, 105.0f, 22.0f, 22.0f } },
		{ ButtonId::Pollution, { 285.0f, 128.0f, 22.0f, 22.0f } },
		{ ButtonId::Crime, { 285.0f, 151.0f, 22.0f, 22.0f } },
		{ ButtonId::Money, { 285.0f, 174.0f, 22.0f, 22.0f } }
	};


	bool ButtonToggled(const ButtonId id)
	{
		for (auto& button : Buttons)
		{
			if (button.id == id)
			{
				return button.toggled;
			}
		}

		return false;
	}
};


GraphWindow::GraphWindow(SDL_Renderer* renderer) :
	mRenderer(*renderer),
	mTexture(loadTexture(renderer, "images/graph.png"))
{
    size({264, 287});
	initTexture(*MainWindowRenderer, mGraphTexture, { static_cast<int>(GraphLayout.w), static_cast<int>(GraphLayout.h) });
}


void GraphWindow::onMoved(const Vector<int>& movement)
{
	GraphPosition = { GraphLayout.x + area().position.x, GraphLayout.y + area().position.y, GraphLayout.w, GraphLayout.h };

	for (auto& button : Buttons)
	{
		button.area = { ButtonLayout.at(button.id).x + area().position.x, ButtonLayout.at(button.id).y + area().position.y, 36, 36 };
		button.iconPosition = { button.area.x + 6, button.area.y + 6, 22, 22 };
	}
}


void GraphWindow::onPositionChanged(const Point<int>& position)
{
	GraphPosition = { GraphLayout.x + area().position.x, GraphLayout.y + area().position.y, GraphLayout.w, GraphLayout.h };

	for (auto& button : Buttons)
	{
		button.area = { ButtonLayout.at(button.id).x + area().position.x, ButtonLayout.at(button.id).y + area().position.y, 36, 36 };
		button.iconPosition = { button.area.x + 6, button.area.y + 6, 22, 22 };
	}
}


void GraphWindow::onMouseDown(const Point<int>& position)
{ 
	const SDL_FPoint& pt{ static_cast<float>(position.x), static_cast<float>(position.y) };

	for (auto& button : Buttons)
	{
		if (SDL_PointInRectFloat(&pt, &button.area))
		{
			button.toggled = !button.toggled;
			update();
			return; // assumption: mouse position can only ever be within one button
		}
	}
}


void GraphWindow::draw()
{
    const SDL_FRect rect{ static_cast<float>(area().position.x), static_cast<float>(area().position.y), static_cast<float>(area().size.x), static_cast<float>(area().size.y)};
	SDL_RenderTexture(&mRenderer, mTexture.texture, &Bg, &rect);

	for (auto& button : Buttons)
	{
		const SDL_FRect& buttonTexture = button.toggled ? ButtonDown : ButtonUp;
		const int offset = button.toggled ? 1 : 0;
		const SDL_FRect iconPosition{ button.iconPosition.x + offset, button.iconPosition.y + offset, button.iconPosition.w, button.iconPosition.h };

		SDL_RenderTexture(&mRenderer, mTexture.texture, &buttonTexture, &button.area);
		SDL_RenderTexture(&mRenderer, mTexture.texture, &IconRects.at(button.id), &iconPosition);
		SDL_RenderTexture(&mRenderer, mGraphTexture.texture, &mGraphTexture.area, &GraphPosition);
	}
}


void GraphWindow::update()
{
	for (auto& [type, graph] : HistoryGraphTable)
	{
		fillGraphPoints(graph.points, graph.history);
	}

	SDL_SetRenderTarget(MainWindowRenderer, mGraphTexture.texture);
	
	turnOffBlending(*MainWindowRenderer, mGraphTexture);

	SDL_SetRenderDrawColor(MainWindowRenderer, 0, 0, 0, 0);
	SDL_RenderClear(MainWindowRenderer);

	for (auto& [type, graph] : HistoryGraphTable)
	{
		if (!ButtonToggled(type)) { continue; }
		SDL_SetRenderDrawColor(MainWindowRenderer, graph.color.r, graph.color.g, graph.color.b, 255);
		SDL_RenderLines(MainWindowRenderer, graph.points.data(), static_cast<int>(graph.points.size()));
	}

	turnOnBlending(*MainWindowRenderer, mGraphTexture);
	SDL_SetRenderTarget(MainWindowRenderer, nullptr);
}


void GraphWindow::fillGraphPoints(Graph::PointsList& points, const GraphHistory& history)
{
	float sx = static_cast<float>(GraphLayout.w / 120.0f);
	float sy = static_cast<float>(GraphLayout.h / 256.0f);

	for (int i = 0; i < HistoryLength; ++i)
	{
		const float x = i * sx;
		const float y = GraphLayout.h - (history[i] * sy);

		points[i] = { x, y };
	}
}
