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

#include "ToggleButton.h"

#include "../Delegate.h"
#include "../Texture.h"

#include "../Math/Point.h"
#include "../Math/Vector.h"

#include <array>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL.h>


class EffectMap;


class MiniMapWindow
{
public:

	enum class ButtonId
	{
		Normal,
		LandValue,
		Crime,
		FireProtection,
		PoliceProtection,
		PopulationDensity,
		PopulationGrowth,
		Pollution,
		TrafficDensity,
		TransportationNetwork,
		PowerGrid,
		Residential,
		Commercial,
		Industrial
	};

	static constexpr auto MiniTileSize{ 3 };
	static constexpr auto TileSize{ 16 };
	static constexpr auto ButtonAreaHeight{ 28 };

	using MapCoordsDelegate = std::function<void(const Point<int>&)>;
	using EffectMapButtonMapping = std::vector<std::tuple<ButtonId, const EffectMap&>>;

public:
	MiniMapWindow() = delete;
	MiniMapWindow(const Point<int>& position, const Vector<int>& size);

	~MiniMapWindow();

	uint32_t id() const;

	void focusOnMapCoordBind(MapCoordsDelegate);

	void updateMapViewPosition(const Point<int>& position);
	void updateViewportSize(const Vector<int>& viewportSize);
	void updateTilePointedAt(const Point<int>& tilePointedAt);

	void linkEffectMap(ButtonId id, const EffectMap& map);
	void linkEffectMaps(EffectMapButtonMapping maps);

	void hide();
	void show();

	bool hidden() const;

	void draw();
	void drawUI();

	void injectEvent(const SDL_Event& event);

private:
	void handleMouseEvent(const SDL_Event& event);
	void handleMouseButtonDown(const SDL_Event& event);
	void handleWindowEvent(const SDL_Event& event);
	void handleMouseMotion(const SDL_Event& event);

	void handleMinimapArea(const Point<int>& point);
	void handleButtonArea(const Point<int>& point);
	void handleNoUiButtonSelected(ButtonId previousButtonDownId);

	void focusViewpoint(const Point<int>& point);

	void setButtonValues();
	void setButtonTextureUv();
	void setButtonPositions();

	bool noButtonsSelected();

	void initTexture(Texture& texture, const Vector<int>& dimensions);
	void initOverlayTextures();

	void drawCurrentOverlay();
    void drawPlainMap();
	void drawResidential();
	void drawCommercial();
	void drawIndustrial();
	void drawPowerMap();
    void drawLilTransMap();

	void fillButtonHandlerTable();

private:
	SDL_Window* mWindow{ nullptr };
	SDL_Renderer* mRenderer{ nullptr };

	Texture mTiles{};
	Texture mTexture{};
	Texture mButtonTextures{};

	SDL_FRect mSelector{};
	SDL_FRect mTileHighlight{ 0.0f, 0.0f, static_cast<float>(MiniTileSize), static_cast<float>(MiniTileSize) };
	SDL_FRect mTileRect{ 0.0f, 0.0f, static_cast<float>(MiniTileSize), static_cast<float>(MiniTileSize) };
	SDL_FRect mMinimapArea{};
	SDL_FRect mButtonArea{};

	uint32_t mWindowID{};

	Vector<int> mMapSize{};

	std::array<SDL_FRect, 28> mButtonUV{};
	std::array<ToggleButton, 14> mButtons{};

	std::unordered_map<ButtonId, Texture> mOverlayTextures;
	std::unordered_map<ButtonId, const EffectMap*> mEffectMaps;

	std::unordered_map<ButtonId, VoidDelegate> mDrawHandlers;

	MapCoordsDelegate mFocusOnTileCallback;

	ButtonId mButtonDownId{ ButtonId::Normal };

	bool mButtonDownInMinimapArea{ false };
	bool mHidden{ true };
};
