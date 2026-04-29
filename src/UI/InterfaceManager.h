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

#include "BudgetWindow.h"
#include "DashboardWindow.h"
#include "EvaluationWindow.h"
#include "FileIoDialog.h"
#include "GraphWindow.h"
#include "MiniMapWindow.h"
#include "OptionsWindow.h"
#include "QueryWindow.h"
#include "ToolPalette.h"
#include "WindowGroup.h"
#include "WindowStack.h"


#include <SDL3/SDL.h>


class InterfaceManager
{
public:
	enum class Window
	{
		Budget,
		Dashboard,
		Graph,
		Evaluation,
		MiniMap,
		ToolPalette,
		Options,
		Query
	};

public:
	InterfaceManager() = delete;
	InterfaceManager(const InterfaceManager&) = delete;
	InterfaceManager& operator=(const InterfaceManager&) = delete;

public:
	InterfaceManager(SDL_Renderer* renderer, SDL_Window* window, Budget& budget, const RCI& rci, const ToolManager& toolManager);

	void injectMouseMotion(const Vector<int>& delta);
	bool injectMouseDown(const Point<int>& position);
	void injectMouseUp();

	void newMonth();

	bool pointInWindow(const Point<int>& position) const;

	void positionWindow(Window window, const Point<int>& position);
	void centerWindow(Window window);
	void centerWindows(const std::vector<Window>& windows);
	void centerAllWindows();

	void showWindow(Window window);

	BudgetWindow& budgetWindow() { return mBudgetWindow; }
	DashboardWindow& dashboardWindow() { return mDashboardWindow; }
	EvaluationWindow& evaluationWindow() { return mEvaluationWindow; }
	FileIoDialog& fileIoDialog() { return mFileIo; }
	OptionsWindow& optionsWindow() { return mOptionsWindow; }
	QueryWindow& queryWindow() { return mQueryWindow; }
	ToolPalette& toolPalette() { return mToolPalette; }

	bool modalWindowVisible() const;

	void hideAllWindows();

	void draw();

private:
	SDL_Renderer* mRenderer{ nullptr };
	SDL_Window* mWindow{ nullptr };

	BudgetWindow mBudgetWindow;
	DashboardWindow mDashboardWindow;
	GraphWindow mGraphWindow;
	EvaluationWindow mEvaluationWindow;
	ToolPalette mToolPalette;
	OptionsWindow mOptionsWindow;
	QueryWindow mQueryWindow;
	FileIoDialog mFileIo;

	WindowStack mWindowStack;
	WindowGroup mModalWindows;
};
