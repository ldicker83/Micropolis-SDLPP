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
#include "MiniMapWindow.h"

#include "../Colors.h"
#include "../EffectMap.h"
#include "../Graphics.h"
#include "../Map.h"

#include "../Util.h"

#include <SDL3/SDL.h>

#if defined(__APPLE__)
#include <SDL3_image/SDL_image.h>
#else
#include <SDL3_image/SDL_image.h>
#endif

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>


namespace
{
    constexpr auto VAL_NONE = 0;
    constexpr auto VAL_LOW = 1;
    constexpr auto VAL_MEDIUM = 2;
    constexpr auto VAL_HIGH = 3;
    constexpr auto VAL_VERYHIGH = 4;
    constexpr auto VAL_PLUS = 5;
    constexpr auto VAL_VERYPLUS = 6;
    constexpr auto VAL_MINUS = 7;
    constexpr auto VAL_VERYMINUS = 8;

    constexpr auto UNPOWERED = 0;
    constexpr auto POWERED = 1;
    constexpr auto CONDUCTIVE = 2;

    constexpr SDL_Color BackgroundColor{ 228, 228, 228, 255 };

    std::array<SDL_Color, 3> PowerColorTable =
    { {
        Colors::LightGrey,
        Colors::Red,
        Colors::LightGrey
    } };


    std::array<SDL_Color, 9> OverlayColorTable =
    { {
        Colors::Clear,
        Colors::LightGrey,
        Colors::Yellow,
        Colors::Orange,
        Colors::Red,
        Colors::DarkGreen,
        Colors::LightGreen,
        Colors::Orange,
        Colors::Yellow
    } };


    int rateOfGrowthColorIndex(int value)
    {
        if (value > 100)
        {
            return VAL_VERYPLUS;
        }

        if (value > 20)
        {
            return VAL_PLUS;
        }

        if (value < -100)
        {
            return VAL_VERYMINUS;
        }

        if (value < -20)
        {
            return VAL_MINUS;
        }

        return VAL_NONE;
    }


    int GetColorIndex(int x)
    {
        if (x < 50)
        {
            return VAL_NONE;
        }
        if (x < 100)
        {
            return VAL_LOW;
        }
        if (x < 150)
        {
            return VAL_MEDIUM;
        }
        if (x < 200)
        {
            return VAL_HIGH;
        }

        return VAL_VERYHIGH;
    }

    void clearOverlayTexture(SDL_Renderer& renderer)
    {
        SDL_SetRenderDrawColor(&renderer, 0, 0, 0, 255);
        SDL_RenderClear(&renderer);
    }

    void drawOverlayPoints(SDL_Renderer& renderer, Texture& overlay, const EffectMap& map)
    {
        SDL_SetRenderTarget(&renderer, overlay.texture);
        turnOffBlending(renderer, overlay);
        clearOverlayTexture(renderer);

        for (int x = 0; x < map.dimensions().x; x++)
        {
            for (int y = 0; y < map.dimensions().y; y++)
            {
                const auto& color = OverlayColorTable[GetColorIndex(map.value({ x, y }))];
                drawPoint(renderer, { x, y }, color);
            }
        }

        turnOnBlending(renderer, overlay);
        SDL_SetRenderTarget(&renderer, nullptr);
    }


    void resetButtonsToNormal(std::array<ToggleButton, 14>& buttons)
    {
		for (auto& button : buttons)
        {
            button.reset();
        }
    }
};


/**
 * Constructs a minimap window
 * 
 * \param position  Position to open the window at
 * \param size      Width/Height of the map in tiles
 */
MiniMapWindow::MiniMapWindow(const Point<int>& position, const Vector<int>& size):
    mMapSize{ size },
    mMinimapArea{ 0.0f, 0.0f, static_cast<float>(size.x * MiniTileSize), static_cast<float>(size.y * MiniTileSize) },
    mButtonArea{ 0.0f, mMinimapArea.h, mMinimapArea.w, static_cast<float>(ButtonAreaHeight) }
{
    if (!SDL_WasInit(SDL_INIT_VIDEO))
    {
        std::cout << "MiniMapWindow::c'tor: SDL Video subsystem was not initialized. Initializing now." << std::endl;
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            throw std::runtime_error(std::string("MiniMapWindow(): Unable to initialize video subsystem: ") + SDL_GetError());
        }
    }

	mWindow = SDL_CreateWindow("Minimap Window",
        size.x * MiniTileSize,
        size.y * MiniTileSize + ButtonAreaHeight,
        SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_UTILITY | SDL_WINDOW_HIDDEN);

	SDL_SetWindowPosition(mWindow, position.x, position.y);

    if (!mWindow)
    {
        throw std::runtime_error("MiniMapWindow(): Unable to create primary window: " + std::string(SDL_GetError()));
    }

	mRenderer = SDL_CreateRenderer(mWindow, nullptr);
    
    if (!mRenderer)
    {
        throw std::runtime_error("MiniMapWindow(): Unable to create renderer: " + std::string(SDL_GetError()));
    }

    mWindowID = SDL_GetWindowID(mWindow);

    mTiles = loadTexture(mRenderer, "images/tilessm.xpm");
    mTexture.texture = SDL_CreateTexture(mRenderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_TARGET, size.x * MiniTileSize, size.y * MiniTileSize);
    mButtonTextures = loadTexture(mRenderer, "icons/minimap.png");

    setButtonValues();
    setButtonTextureUv();
    setButtonPositions();
    resetButtonsToNormal(mButtons);

    initOverlayTextures();
    fillButtonHandlerTable();
}


MiniMapWindow::~MiniMapWindow()
{
    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
}


Uint32 MiniMapWindow::id() const
{
    return mWindowID;
}


void MiniMapWindow::focusOnMapCoordBind(MapCoordsDelegate delegate)
{
    mFocusOnTileCallback = delegate;
}


void MiniMapWindow::updateMapViewPosition(const Point<int>& position)
{
    const Point<int> selectorPosition = position.skewInverseBy({ TileSize, TileSize }).skewBy({ MiniTileSize, MiniTileSize });
    const Vector<int> selectorSize{ static_cast<int>(mSelector.w), static_cast<int>(mSelector.h) };
    const Vector<int> selectorMaxPosition = mMapSize.skewBy({ MiniTileSize, MiniTileSize }) - selectorSize;

    mSelector = {
        static_cast<float>(std::clamp(selectorPosition.x, 0, selectorMaxPosition.x)),
        static_cast<float>(std::clamp(selectorPosition.y, 0, selectorMaxPosition.y)),
        mSelector.w,
        mSelector.h
	};
}


void MiniMapWindow::updateViewportSize(const Vector<int>& viewportSize)
{
    mSelector.w = std::ceil(viewportSize.x / static_cast<float>(TileSize)) * MiniTileSize;
    mSelector.h = std::ceil(viewportSize.y / static_cast<float>(TileSize)) * MiniTileSize;
}


void MiniMapWindow::updateTilePointedAt(const Point<int>& tilePointedAt)
{
    mTileHighlight.x = static_cast<float>(tilePointedAt.x * MiniTileSize);
    mTileHighlight.y = static_cast<float>(tilePointedAt.y * MiniTileSize);
}


void MiniMapWindow::linkEffectMap(ButtonId id, const EffectMap& map)
{
    mEffectMaps[id] = &map;
}


void MiniMapWindow::linkEffectMaps(MiniMapWindow::EffectMapButtonMapping maps)
{
    for(auto& [id, map] : maps)
    {
        linkEffectMap(id, map);
	}
}


void MiniMapWindow::initTexture(Texture& texture, const Vector<int>& dimensions)
{
    texture = {
        SDL_CreateTexture(mRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, dimensions.x, dimensions.y),
        { 0, 0, static_cast<float>(dimensions.x), static_cast<float>(dimensions.y) },
        dimensions
    };

	SDL_SetTextureScaleMode(texture.texture, SDL_SCALEMODE_NEAREST);
}


void MiniMapWindow::initOverlayTextures()
{
    const auto overlayHalfSize = mMapSize.skewInverseBy({ 2, 2 });
    initTexture(mOverlayTextures[ButtonId::Crime], overlayHalfSize);
    initTexture(mOverlayTextures[ButtonId::LandValue], overlayHalfSize);
    initTexture(mOverlayTextures[ButtonId::PopulationDensity], overlayHalfSize);
    initTexture(mOverlayTextures[ButtonId::Pollution], overlayHalfSize);
    initTexture(mOverlayTextures[ButtonId::TrafficDensity], overlayHalfSize);
    
    // Necessary because of integer math dropping fractional component.
    const Vector<int> overlayEigthSize{ mMapSize.x / 8, (mMapSize.y / 8) + 1 };
    initTexture(mOverlayTextures[ButtonId::PoliceProtection], overlayEigthSize);
    initTexture(mOverlayTextures[ButtonId::FireProtection], overlayEigthSize);
    initTexture(mOverlayTextures[ButtonId::PopulationGrowth], overlayEigthSize);
    
    // Full size textures
    initTexture(mOverlayTextures[ButtonId::TransportationNetwork], mMapSize.skewBy({MiniTileSize, MiniTileSize}));
    initTexture(mOverlayTextures[ButtonId::PowerGrid], mMapSize.skewBy({MiniTileSize, MiniTileSize}));
    initTexture(mOverlayTextures[ButtonId::Residential], mMapSize.skewBy({ MiniTileSize, MiniTileSize }));
    initTexture(mOverlayTextures[ButtonId::Commercial], mMapSize.skewBy({ MiniTileSize, MiniTileSize }));
    initTexture(mOverlayTextures[ButtonId::Industrial], mMapSize.skewBy({ MiniTileSize, MiniTileSize }));

}


void MiniMapWindow::drawCurrentOverlay()
{
    if (mButtonDownId != ButtonId::Normal)
    {
        auto map = mEffectMaps[mButtonDownId];
        if (map)
        {
            drawOverlayPoints(*mRenderer, mOverlayTextures[mButtonDownId], *map);
        }
    }
}


void MiniMapWindow::hide()
{
    SDL_HideWindow(mWindow);
    mHidden = true;
}


void MiniMapWindow::show()
{
    SDL_RestoreWindow(mWindow);
    SDL_ShowWindow(mWindow);
    mHidden = false;
}


bool MiniMapWindow::hidden() const
{
    return mHidden;
}


void MiniMapWindow::drawPlainMap()
{
    SDL_FRect miniMapDrawRect{ 0.0f, 0.0f, static_cast<float>(MiniTileSize), static_cast<float>(MiniTileSize) };
    SDL_SetRenderTarget(mRenderer, mTexture.texture);
        
    for (int row = 0; row < mMapSize.x; row++)
    {
        for (int col = 0; col < mMapSize.y; col++)
        {
            miniMapDrawRect = {
                static_cast<float>(row * MiniTileSize),
                static_cast<float>(col * MiniTileSize),
                miniMapDrawRect.w,
                miniMapDrawRect.h
            };
            
            mTileRect.y = static_cast<float>(maskedTileValue(tileValue(row, col)) * MiniTileSize);
            SDL_RenderTexture(mRenderer, mTiles.texture, &mTileRect, &miniMapDrawRect);
        }
    }

    SDL_RenderPresent(mRenderer);
    SDL_SetRenderTarget(mRenderer, nullptr);
}


void MiniMapWindow::drawResidential()
{
    SDL_FRect miniMapDrawRect{ 0.0f, 0.0f, static_cast<float>(MiniTileSize), static_cast<float>(MiniTileSize) };
    SDL_SetRenderTarget(mRenderer, mOverlayTextures[ButtonId::Residential].texture);

    for (int row = 0; row < mMapSize.x; row++)
    {
        for (int col = 0; col < mMapSize.y; col++)
        {
            miniMapDrawRect = {
                static_cast<float>(row * MiniTileSize),
                static_cast<float>(col * MiniTileSize),
                miniMapDrawRect.w,
                miniMapDrawRect.h
            };

            unsigned int tile = maskedTileValue(row, col);

            if (tile > 422)
            {
                tile = 0;
            }

            mTileRect.y = static_cast<float>(maskedTileValue(tile) * MiniTileSize);
            SDL_RenderTexture(mRenderer, mTiles.texture, &mTileRect, &miniMapDrawRect);
        }
    }

    SDL_RenderPresent(mRenderer);
    SDL_SetRenderTarget(mRenderer, nullptr);
}


void MiniMapWindow::drawCommercial()
{
    SDL_FRect miniMapDrawRect{ 0.0f, 0.0f, static_cast<float>(MiniTileSize), static_cast<float>(MiniTileSize) };
    SDL_SetRenderTarget(mRenderer, mOverlayTextures[ButtonId::Commercial].texture);

    for (int row = 0; row < mMapSize.x; row++)
    {
        for (int col = 0; col < mMapSize.y; col++)
        {
            miniMapDrawRect = {
                static_cast<float>(row * MiniTileSize),
                static_cast<float>(col * MiniTileSize),
                miniMapDrawRect.w,
                miniMapDrawRect.h
            };

            unsigned int tile = maskedTileValue(row, col);

            if ((tile > 609) || ((tile >= 232) && (tile < 423)))
            {
                tile = 0;
            }

            mTileRect.y = static_cast<float>(maskedTileValue(tile) * MiniTileSize);
            SDL_RenderTexture(mRenderer, mTiles.texture, &mTileRect, &miniMapDrawRect);
        }
    }

    SDL_RenderPresent(mRenderer);
    SDL_SetRenderTarget(mRenderer, nullptr);
}


void MiniMapWindow::drawIndustrial()
{
    SDL_FRect miniMapDrawRect{ 0.0f, 0.0f, static_cast<float>(MiniTileSize), static_cast<float>(MiniTileSize) };
    SDL_SetRenderTarget(mRenderer, mOverlayTextures[ButtonId::Industrial].texture);

    for (int row = 0; row < mMapSize.x; row++)
    {
        for (int col = 0; col < mMapSize.y; col++)
        {
            miniMapDrawRect = {
                static_cast<float>(row * MiniTileSize),
                static_cast<float>(col * MiniTileSize),
                miniMapDrawRect.w,
                miniMapDrawRect.h
            };

            unsigned int tile = maskedTileValue(row, col);

            if (((tile >= 240) && (tile <= 611)) ||
                ((tile >= 693) && (tile <= 851)) ||
                ((tile >= 860) && (tile <= 883)) ||
                (tile >= 932))
            {
                tile = 0;
            }

            mTileRect.y = static_cast<float>(maskedTileValue(tile) * MiniTileSize);
            SDL_RenderTexture(mRenderer, mTiles.texture, &mTileRect, &miniMapDrawRect);
        }
    }

    SDL_RenderPresent(mRenderer);
    SDL_SetRenderTarget(mRenderer, nullptr);
}


void MiniMapWindow::drawPowerMap()
{
    SDL_Color tileColor{};
    SDL_FRect miniMapDrawRect{ 0.0f, 0.0f, static_cast<float>(MiniTileSize), static_cast<float>(MiniTileSize) };
    SDL_SetRenderTarget(mRenderer, mOverlayTextures[ButtonId::PowerGrid].texture);

    for (int row = 0; row < mMapSize.x; row++)
    {
        for (int col = 0; col < mMapSize.y; col++)
        {
            miniMapDrawRect = {
                static_cast<float>(row * MiniTileSize),
                static_cast<float>(col * MiniTileSize),
                miniMapDrawRect.w,
                miniMapDrawRect.h
            };

            const unsigned int unmaskedTile = tileValue(row, col);
            unsigned int tile = maskedTileValue(unmaskedTile);

            bool colored{ true };

            if (tile <= FireLast)
            {
                colored = false;
            }
            else if (unmaskedTile & ZonedBit)
            {
                tileColor = (unmaskedTile & PowerBit) ? Colors::Red : Colors::LightBlue;
            }
            else
            {
                if (unmaskedTile & ConductiveBit)
                {
                    tileColor = Colors::LightGrey;
                }
                else
                {
                    tile = Dirt;
                    colored = false;
                }
            }

            if (colored)
            {
                SDL_SetRenderDrawColor(mRenderer, tileColor.r, tileColor.g, tileColor.b, 255);
                SDL_RenderFillRect(mRenderer, &miniMapDrawRect);
            }
            else
            {
                mTileRect.y = static_cast<float>(maskedTileValue(tileValue(row, col)) * MiniTileSize);
                SDL_RenderTexture(mRenderer, mTiles.texture, &mTileRect, &miniMapDrawRect);
            }
        }
    }

    SDL_RenderPresent(mRenderer);
    SDL_SetRenderTarget(mRenderer, nullptr);
}


void MiniMapWindow::drawLilTransMap()
{
    SDL_FRect miniMapDrawRect{ 0.0f, 0.0f, static_cast<float>(MiniTileSize), static_cast<float>(MiniTileSize) };
    SDL_SetRenderTarget(mRenderer, mOverlayTextures[ButtonId::TransportationNetwork].texture);
    
    for (int row = 0; row < mMapSize.x; row++)
    {
        for (int col = 0; col < mMapSize.y; col++)
        {
            miniMapDrawRect = {
                static_cast<float>(row * MiniTileSize),
                static_cast<float>(col * MiniTileSize),
                miniMapDrawRect.w,
                miniMapDrawRect.h
            };

            unsigned int tile = maskedTileValue(row, col);

            if ((tile >= ResidentialBase) ||
                ((tile >= BRWXXX7) && tile <= 220) ||
                (tile == Unused6))
            {
                tile = 0;
            }

            mTileRect.y = static_cast<float>(maskedTileValue(tile) * MiniTileSize);
            SDL_RenderTexture(mRenderer, mTiles.texture, &mTileRect, &miniMapDrawRect);
        }
    }

    SDL_RenderPresent(mRenderer);
    SDL_SetRenderTarget(mRenderer, nullptr);
}


void MiniMapWindow::fillButtonHandlerTable()
{
    mDrawHandlers.emplace(ButtonId::Normal, [this]() { drawPlainMap(); });
    mDrawHandlers.emplace(ButtonId::LandValue, [this]() { drawCurrentOverlay(); });
    mDrawHandlers.emplace(ButtonId::Crime, [this]() { drawCurrentOverlay(); });
    mDrawHandlers.emplace(ButtonId::FireProtection, [this]() { drawCurrentOverlay(); });
    mDrawHandlers.emplace(ButtonId::PoliceProtection, [this]() { drawCurrentOverlay(); });
    mDrawHandlers.emplace(ButtonId::PopulationDensity, [this]() { drawCurrentOverlay(); });
    mDrawHandlers.emplace(ButtonId::PopulationGrowth, [this]() { drawCurrentOverlay(); });
    mDrawHandlers.emplace(ButtonId::Pollution, [this]() { drawCurrentOverlay(); });
    mDrawHandlers.emplace(ButtonId::TrafficDensity, [this]() { drawCurrentOverlay(); });
    mDrawHandlers.emplace(ButtonId::TransportationNetwork, [this]() { drawLilTransMap(); });
    mDrawHandlers.emplace(ButtonId::PowerGrid, [this]() { drawPowerMap(); });
    mDrawHandlers.emplace(ButtonId::Residential, [this]() { drawResidential(); });
    mDrawHandlers.emplace(ButtonId::Commercial, [this]() { drawCommercial(); });
    mDrawHandlers.emplace(ButtonId::Industrial, [this]() { drawIndustrial(); });
}


void MiniMapWindow::draw()
{
    switch (mButtonDownId)
    {
    case ButtonId::PowerGrid:
        drawPowerMap();
        return;

    case ButtonId::TransportationNetwork:
        drawLilTransMap();
        return;

    case ButtonId::Residential:
        drawResidential();
        return;

    case ButtonId::Commercial:
        drawCommercial();
        return;

    case ButtonId::Industrial:
        drawIndustrial();
        return;

    default:
        drawPlainMap();
        drawCurrentOverlay();
    }
}


void MiniMapWindow::drawUI()
{
    SDL_SetRenderDrawColor(mRenderer, BackgroundColor.r, BackgroundColor.g, BackgroundColor.b, 255);
    SDL_RenderClear(mRenderer);

    SDL_RenderTexture(mRenderer, mTexture.texture, nullptr, &mMinimapArea);

    if (mButtonDownId != ButtonId::Normal)
    {
        SDL_RenderTexture(mRenderer, mOverlayTextures[mButtonDownId].texture, nullptr, &mMinimapArea);
    }

    SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 150);
    SDL_RenderRect(mRenderer, &mSelector);
    SDL_RenderRect(mRenderer, &mTileHighlight);

    const int arraySize = static_cast<int>(mButtons.size());
    for (int i{ 0 }; i < arraySize; ++i)
    {
		const int textureColumn = mButtons[i].toggled() ? 1 : 0;
		const auto buttonArea = mButtons[i].areaF();
        SDL_RenderTexture(mRenderer, mButtonTextures.texture, &mButtonUV[i + (textureColumn * static_cast<size_t>(arraySize))], &buttonArea);
    }

    SDL_RenderPresent(mRenderer);
}


void MiniMapWindow::injectEvent(const SDL_Event& event)
{
    if (event.window.windowID != id())
    {
        return;
    }

    if (event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST)
    {
        handleWindowEvent(event);
        return;
    }

    switch (event.type)
    {
    case SDL_EVENT_MOUSE_MOTION:
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        handleMouseEvent(event);
        break;

    default:
        break;
    }
}


void MiniMapWindow::handleMouseEvent(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        handleMouseButtonDown(event);
        break;

    case SDL_EVENT_MOUSE_MOTION:
        handleMouseMotion(event);
        break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        mButtonDownInMinimapArea = false;
        break;

    default:
        break;
    }
}


void MiniMapWindow::handleMouseButtonDown(const SDL_Event& event)
{
    if (event.button.button != SDL_BUTTON_LEFT)
    {
        return;
    }

    const Point<int> point{ static_cast<int>(event.button.x), static_cast<int>(event.button.y) };
    if (pointInFRect(point, mMinimapArea))
    {
        handleMinimapArea(point);
    }
    else if (pointInFRect(point, mButtonArea))
    {
        handleButtonArea(point);
    }   
}


void MiniMapWindow::handleWindowEvent(const SDL_Event& event)
{
    switch (event.window.type)
    {
    case SDL_EVENT_WINDOW_MINIMIZED:
        hide();
        mButtonDownInMinimapArea = false;
        break;

    case SDL_EVENT_WINDOW_FOCUS_LOST:
    case SDL_EVENT_WINDOW_HIDDEN:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        mButtonDownInMinimapArea = false;
        break;

    default:
        break;
    }
}


void MiniMapWindow::handleMouseMotion(const SDL_Event& event)
{
    if (!mButtonDownInMinimapArea)
    {
        return;
    }

    if (event.motion.state & SDL_BUTTON_LMASK)
    {
        focusViewpoint({
            static_cast<int>(event.motion.x),
            static_cast<int>(event.motion.y)
            });
    }
}


void MiniMapWindow::handleMinimapArea(const Point<int>& point)
{
    mButtonDownInMinimapArea = true;
    focusViewpoint(point);
}


void MiniMapWindow::handleButtonArea(const Point<int>& point)
{
    auto previousButtonDownId = mButtonDownId;

    for (auto& button : mButtons)
    {
        if (!pointInRect(point, button.area()))
        {
            button.reset();
            continue;
        }

        button.toggled(true);
        mButtonDownId = static_cast<ButtonId>(button.userValue());
        mDrawHandlers.at(mButtonDownId)();
    }

    handleNoUiButtonSelected(previousButtonDownId);
}


void MiniMapWindow::handleNoUiButtonSelected(ButtonId previousButtonDownId)
{
    if (noButtonsSelected())
    {
        mButtons[static_cast<size_t>(previousButtonDownId)].toggled(true);
        mButtonDownId = previousButtonDownId;
    }
}


void MiniMapWindow::focusViewpoint(const Point<int>& point)
{
	const Vector<int> selectorSize{ static_cast<int>(mSelector.w) / 2, static_cast<int>(mSelector.h) / 2 };
	const Point<int> adjustedPosition{ point - selectorSize };

	updateMapViewPosition(adjustedPosition.skewBy({ TileSize, TileSize }).skewInverseBy({ MiniTileSize, MiniTileSize }));

	if (mFocusOnTileCallback)
	{
		const Point<int> position{ static_cast<int>(mSelector.x), static_cast<int>(mSelector.y) };
		mFocusOnTileCallback(position);
	}
}


void MiniMapWindow::setButtonValues()
{
    mButtons[0].userValue(static_cast<int>(ButtonId::Normal));
    mButtons[1].userValue(static_cast<int>(ButtonId::LandValue));
    mButtons[2].userValue(static_cast<int>(ButtonId::Crime));
    mButtons[3].userValue(static_cast<int>(ButtonId::FireProtection));
    mButtons[4].userValue(static_cast<int>(ButtonId::PoliceProtection));
    mButtons[5].userValue(static_cast<int>(ButtonId::PopulationDensity));
    mButtons[6].userValue(static_cast<int>(ButtonId::PopulationGrowth));
    mButtons[7].userValue(static_cast<int>(ButtonId::Pollution));
    mButtons[8].userValue(static_cast<int>(ButtonId::TrafficDensity));
    mButtons[9].userValue(static_cast<int>(ButtonId::TransportationNetwork));
    mButtons[10].userValue(static_cast<int>(ButtonId::PowerGrid));
    mButtons[11].userValue(static_cast<int>(ButtonId::Residential));
    mButtons[12].userValue(static_cast<int>(ButtonId::Commercial));
    mButtons[13].userValue(static_cast<int>(ButtonId::Industrial));
}


void MiniMapWindow::setButtonTextureUv()
{
    const int arraySize = static_cast<int>(mButtons.size());
    for (int i = 0; i < arraySize * 2; ++i)
    {
        mButtonUV[i] = { static_cast<float>((i / arraySize) * 24), static_cast<float>((i % arraySize) * 24), 24.0f, 24.0f };
    }
}


void MiniMapWindow::setButtonPositions()
{
    constexpr Vector<float> buttonSize{ 24.0f, 24.0f };
    constexpr Vector<float> buttonTransform{ buttonSize.x + 1.0f, 0.0f };
    
    const int arraySize = static_cast<int>(mButtons.size());
    const float startPosition = static_cast<float>((mButtonArea.w - (buttonTransform.x * arraySize)) / 2);

    for (int i{ 0 }; i < arraySize; ++i)
    {
        mButtons[i].areaF({ startPosition + buttonTransform.x * i, mButtonArea.y + 3, buttonSize.x, buttonSize.y });
    }
}


bool MiniMapWindow::noButtonsSelected()
{
    for (auto& button : mButtons)
    {
        if (button.toggled())
        {
            return false;
        }
    }
    
    return true;
}
