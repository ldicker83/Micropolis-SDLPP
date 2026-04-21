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
#include "WindowBase.h"

#include <SDL3/SDL.h>


namespace
{
    constexpr int TitleBarHeight = 20;
    constexpr int ClientAreaPadding = 8;
    constexpr Vector<int> CloseButtonSize{ 13, 13 };
    constexpr Vector<int> CloseButtonPosition{ 4, 4 };
}


bool WindowBase::visible() const
{
    return mVisible;
}


void WindowBase::toggleVisible()
{
    if (mAlwaysVisible)
    {
		mVisible = true;
        return;
    }

    mVisible = !mVisible;
    mVisible ? show() : hide();
}


void WindowBase::show()
{
    mVisible = true;
    onShow();
}


void WindowBase::hide()
{
    if(mAlwaysVisible)
    {
        return;
	}

    mVisible = false;
    onHide();
}


void WindowBase::move(const Vector<int>& delta)
{
    mArea.startPoint(mArea.startPoint() + delta);
    mTitleBarArea.startPoint(mArea.startPoint());
    mClientArea.startPoint(mArea.startPoint() + Vector<int>{ClientAreaPadding, TitleBarHeight});
    mCloseButtonArea.startPoint(mArea.startPoint() + CloseButtonPosition);

    onMoved(delta);
}


void WindowBase::position(const Point<int>& point)
{
    mArea.startPoint(point);
    mTitleBarArea.startPoint(point);
    mClientArea.startPoint(point + Vector<int>{ClientAreaPadding, TitleBarHeight});
	mCloseButtonArea.startPoint(point + CloseButtonPosition);
    onPositionChanged(point);
}


constexpr Point<int> WindowBase::position() const
{
    return mArea.startPoint();
}


void WindowBase::size(const Vector<int>& size)
{
    mArea.size = size;
    mTitleBarArea.size = { size.x, TitleBarHeight };
    mClientArea.size = size - Vector<int>{ ClientAreaPadding * 2, TitleBarHeight + ClientAreaPadding };
    mCloseButtonArea.size = CloseButtonSize;
}


const Rectangle<int>& WindowBase::area() const
{
    return mArea;
}


const Rectangle<int>& WindowBase::clientArea() const
{
    return mClientArea;
}


void WindowBase::closeButtonActive(bool show)
{
    mCloseButtonActive = show;
}


void WindowBase::anchor()
{
    mAnchored = true;
}


void WindowBase::unanchor()
{
    mAnchored = false;
}


bool WindowBase::anchored() const
{
    return mAnchored;
}


void WindowBase::alwaysVisible(bool always)
{
    mAlwaysVisible = always;
}


bool WindowBase::alwaysVisible() const
{
    return mAlwaysVisible;
}


void WindowBase::injectMouseDown(const Point<int>& position)
{
    const SDL_Point& pt{ position.x, position.y };

    const SDL_Rect closeButton{ mCloseButtonArea.position.x, mCloseButtonArea.position.y, mCloseButtonArea.size.x, mCloseButtonArea.size.y };
    if (mCloseButtonActive && SDL_PointInRect(&pt, &closeButton))
    {
        hide();
        return;
    }

    const SDL_Rect titlebar{ mTitleBarArea.position.x, mTitleBarArea.position.y, mTitleBarArea.size.x, mTitleBarArea.size.y };
    if (SDL_PointInRect(&pt, &titlebar) && !anchored())
    {
        mDragging = true;
        return;
    }

    onMouseDown(position);
}


void WindowBase::injectMouseUp()
{
    mDragging = false;
    onMouseUp();
}


void WindowBase::injectMouseMotion(const Vector<int>& delta)
{
    if (!mDragging) { return; }
    move(delta);
}


void WindowBase::injectKeyDown(int32_t key)
{
    onKeyDown(key);
}
