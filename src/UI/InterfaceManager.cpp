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
#include "InterfaceManager.h"

#include <unordered_map>

namespace
{

	std::unordered_map<InterfaceManager::Window, WindowBase*> WindowTable;


	void CenterWindow(SDL_Window* sdlWindow, WindowBase& window)
	{
		int windowWidth = 0, windowHeight = 0;
		SDL_GetWindowSize(sdlWindow, &windowWidth, &windowHeight);
		window.position({ windowWidth / 2 - window.area().size.x / 2, windowHeight / 2 - window.area().size.y / 2 });
	}


	void BringWindowToFront(WindowStack& stack, WindowBase& window)
	{
		stack.bringToFront(&window);
		window.toggleVisible();
		
		if(window.visible())
		{
			window.update();
		}
	}
}

InterfaceManager::InterfaceManager(SDL_Renderer* renderer, SDL_Window* window, Budget& budget, const RCI& rci) :
	mRenderer{ renderer },
	mWindow{ window },
	mBudgetWindow{ renderer, budget },
	mDashboardWindow{ renderer, budget, rci },
	mGraphWindow{ renderer },
	mEvaluationWindow{ renderer },
	mToolPalette{ renderer },
	mOptionsWindow{ renderer },
	mQueryWindow{ renderer },
	mFileIo{ *window }
{
	mWindowStack.addWindow(&mBudgetWindow);
	mWindowStack.addWindow(&mDashboardWindow);
	mWindowStack.addWindow(&mEvaluationWindow);
	mWindowStack.addWindow(&mGraphWindow);
	mWindowStack.addWindow(&mOptionsWindow);
	mWindowStack.addWindow(&mQueryWindow);
	mWindowStack.addWindow(&mToolPalette);
	

	mModalWindows.addWindow(&mBudgetWindow);
	mModalWindows.addWindow(&mOptionsWindow);

	WindowTable[InterfaceManager::Window::Budget] = &mBudgetWindow;
	WindowTable[InterfaceManager::Window::Dashboard] = &mDashboardWindow;
	WindowTable[InterfaceManager::Window::Evaluation] = &mEvaluationWindow;
	WindowTable[InterfaceManager::Window::Graph] = &mGraphWindow;
	WindowTable[InterfaceManager::Window::Options] = &mOptionsWindow;
	WindowTable[InterfaceManager::Window::Query] = &mQueryWindow;
	WindowTable[InterfaceManager::Window::ToolPalette] = &mToolPalette;

	centerAllWindows();
}


void InterfaceManager::injectMouseMotion(const Vector<int>& delta)
{
	mWindowStack.injectMouseMotion(delta);
}


/**
 * Injects a mouse down event into the InterfaceManager.
 * 
 * \return True if the event was consumed.
 **/
bool InterfaceManager::injectMouseDown(const Point<int>& position)
{
	if (mWindowStack.pointInWindow(position))
	{
		mWindowStack.updateStack(position);
		mWindowStack.front()->injectMouseDown(position);
		return true;
	}

	return false;
}


void InterfaceManager::injectMouseUp()
{
	mWindowStack.injectMouseUp();
}


void InterfaceManager::newMonth()
{
	if (mGraphWindow.visible())
	{
		mGraphWindow.update();
	}
}


bool InterfaceManager::pointInWindow(const Point<int>& position) const
{
	return mWindowStack.pointInWindow(position);
}


void InterfaceManager::positionWindow(Window window, const Point<int>& position)
{
	WindowTable.at(window)->position(position);
}


void InterfaceManager::centerWindow(Window window)
{
	CenterWindow(mWindow, *WindowTable.at(window));
}


void InterfaceManager::centerWindows(const std::vector<Window>& windows)
{
	for (auto window : windows)
	{
		centerWindow(window);
	}
}


void InterfaceManager::centerAllWindows()
{
	for (auto& [id, window] : WindowTable)
	{
		CenterWindow(mWindow, *window);
	}
}


void InterfaceManager::showWindow(Window window)
{
	auto windowPtr = WindowTable.at(window);
	BringWindowToFront(mWindowStack, *windowPtr);
	windowPtr->show();
}


bool InterfaceManager::modalWindowVisible() const
{
	return mModalWindows.windowVisible();
}


void InterfaceManager::hideAllWindows()
{
	mWindowStack.hide();
	mModalWindows.hide();
}


void InterfaceManager::draw()
{
	if (mModalWindows.windowVisible())
	{
		SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 175);
		SDL_RenderFillRect(mRenderer, nullptr);

		mModalWindows.draw();
	}
	else
	{
		mWindowStack.draw();
	}
}
