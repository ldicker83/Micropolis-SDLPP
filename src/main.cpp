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
#include "main.h"

#include "Budget.h"
#include "CityProperties.h"
#include "Colors.h"
#include "Connection.h"
#include "Evaluation.h"
#include "FileIo.h"
#include "Font.h"
#include "g_ani.h"
#include "GameOptions.h"
#include "Graph.h"
#include "Map.h"
#include "s_alloc.h"
#include "s_disast.h"
#include "s_gen.h"
#include "s_msg.h"
#include "s_sim.h"
#include "Scan.h"
#include "Sprite.h"
#include "StringRender.h"
#include "Texture.h"
#include "Tool.h"
#include "Util.h"
#include "w_sound.h"
#include "w_tk.h"
#include "w_update.h"

#include "UI/InterfaceManager.h"
#include "UI/MiniMapWindow.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if defined(__APPLE__)
#include <SDL3_image/SDL_image.h>
#else
//#include <SDL3/SDL_image.h>
#include <SDL3_image/SDL_Image.h>
#endif

#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN
#endif

const std::string MicropolisVersion = "4.0";

SDL_Window* MainWindow = nullptr;
SDL_Renderer* MainWindowRenderer = nullptr;

uint32_t MainWindowId{};

Texture MainMapTexture{};

Texture BigTileset{};


int InitSimLoad;
int ScenarioID;


namespace
{
    constexpr auto TileSize = 16;
    constexpr auto MiniTileSize = 3;
    constexpr auto MiniMapTileMultiplier = (TileSize + MiniTileSize - 1) / MiniTileSize;

	constexpr Point<int> DashboardWindowDefaultPosition{ 10, 10 };
	constexpr Point<int> ToolPaletteDefaultPosition{ 10, 100 };

    SDL_FRect UiHeaderRect{ 10.0f, 10.0f, 0.0f, 0.0f };
    SDL_FRect FullMapViewRect{};

    Vector<int> WindowSize{};
    Vector<int> DraggableToolVector{};

    Point<int> MapViewOffset{};
    Point<int> TilePointedAt{};

    bool Exit{ false };
    bool RedrawMinimap{ false };
    bool SimulationStep{ false };
    bool AnimationStep{ false };
    bool RightButtonDrag{ false };

    GameOptions gameOptions;

    constexpr unsigned int SimStepDefaultTime{ 100 };
    constexpr unsigned int AnimationStepDefaultTime{ 150 };

    SDL_Rect TileHighlight{ 0, 0, TileSize, TileSize };

    std::array<unsigned int, 5> SpeedModifierTable{ 0, 0, 50, 75, 95 };

    std::string currentBudget{};

    std::vector<SDL_TimerID> Timers;

    Budget budget{};
    CityProperties cityProperties{};

    std::unique_ptr<MiniMapWindow> miniMapWindow;

    std::unique_ptr<StringRender> stringRenderer;

    std::unique_ptr<Font> MainFont;

	std::unique_ptr<InterfaceManager> interfaceManager;

    unsigned int speedModifier()
    {
        return SpeedModifierTable[static_cast<unsigned int>(simSpeed())];
    }

    Uint32 SDLCALL zonePowerBlinkTick(void*, SDL_TimerID timerID, Uint32 interval)
    {
        toggleBlinkFlag();
        return interval;
    }

    Uint32 SDLCALL redrawMiniMapTick(void*, SDL_TimerID timerID, Uint32 interval)
    {
        RedrawMinimap = true;
        return interval;
    }

    Uint32 SDLCALL simulationTick(void*, SDL_TimerID timerID, Uint32 interval)
    {
        SimulationStep = true;
        return SimStepDefaultTime - speedModifier();
    }

    Uint32 SDLCALL animationTick(void*, SDL_TimerID timerID, Uint32 interval)
    {
        AnimationStep = true;
        return interval;
    }

    void initTimers()
    {
        Timers.push_back(SDL_AddTimer(500, zonePowerBlinkTick, nullptr));
        Timers.push_back(SDL_AddTimer(1000, redrawMiniMapTick, nullptr));
        Timers.push_back(SDL_AddTimer(SimStepDefaultTime, simulationTick, nullptr));
        Timers.push_back(SDL_AddTimer(AnimationStepDefaultTime, animationTick, nullptr));
    }

    void deinitTimers()
    {
        for (auto timer : Timers)
        {
            SDL_RemoveTimer(timer);
        }
    }

    void showBudgetIfNeeded()
    {
        if (!gameOptions.autoBudget && budget.NeedsAttention())
        {
			interfaceManager->showWindow(InterfaceManager::Window::Budget);
        }
    }
};


namespace EventHandling
{
    Point<int> MouseDownPosition{};
    Point<int> MouseClickPosition{};
    Point<int> MousePosition{};

    bool MouseLeftDown{ false };
};


const Point<int>& viewOffset()
{
    return MapViewOffset;
}


void showBudgetWindow()
{
    interfaceManager->showWindow(InterfaceManager::Window::Budget);
}


void simExit()
{
    Exit = true;
}


void simUpdate()
{
    updateDate();

    if (newMonth())
    {
        interfaceManager->newMonth();
    }

    showBudgetIfNeeded();

    refreshCityEvaluation(cityProperties);
}


void simLoop(bool doSim)
{
    if (doSim)
    {
        SimFrame(cityProperties, budget);
        SimulationStep = false;
    }

    if (AnimationStep)
    {
        AnimationStep = false;

        if (!paused())
        {
            animateTiles();
            updateSprites();
        }

        const Point<int> begin{ MapViewOffset.x / TileSize, MapViewOffset.y / TileSize };
        const Point<int> end
        {
            std::clamp((MapViewOffset.x + WindowSize.x) / TileSize + 1, 0, SimWidth),
            std::clamp((MapViewOffset.y + WindowSize.y) / TileSize + 1, 0, SimHeight)
        };

        drawBigMapSegment(begin, end);
    }

    if (RedrawMinimap)
    {
        miniMapWindow->draw();
        RedrawMinimap = false;
    }

    simUpdate();
}


void initWillStuff()
{
    RoadEffect = 32;
    PoliceEffect = 1000;
    FireEffect = 1000;
    cityScore(500);
    cityPopulation(-1);
    lastCityTime(-1);
    lastCityYear(1);
    lastCityMonth(0);
    pendingTool(Tool::None);
    MessageId(NotificationId::None);
    destroyAllSprites();
    DisasterEvent = 0;
    initMapArrays();
}


GameOptions& gameplayOptions()
{
    return gameOptions;
}


void simInit()
{
    userSoundOn(true);

    ScenarioID = 0;
    StartingYear = 1900;
    AutoGotoMessageLocation(true);
    CityTime = 50;
    gameOptions = GameOptions{};
    MessageId(NotificationId::None);
    ClearMes();
    simSpeed(SimulationSpeed::Normal);
    ChangeEval();
    MessageLocation({ 0, 0 });
    
    InitSimLoad = 2;
    Exit = false;

    InitializeSound();
    StopEarthquake();
    ResetMap();
    initWillStuff();
    budget.CurrentFunds(5000);
    setGameLevelFunds(0, cityProperties, budget);
    simSpeed(SimulationSpeed::Paused);
}


void doPlayNewCity(CityProperties& properties, Budget& budget)
{
    GenerateNewCity(properties, budget);
    resume();
    simSpeed(SimulationSpeed::Normal);
}


void doStartScenario(int scenario)
{
    Eval("UIStartScenario " + std::to_string(scenario));
}


void primeGame(const int startFlag, CityProperties& properties, Budget& budget)
{
    switch (startFlag)
    {
    case -2: // Load a city
        if (LoadCity("filename", properties, budget))
        {
			interfaceManager->dashboardWindow().cityName(properties.CityName());
            break;
        }
        // If load fails, simply create a new city
        [[fallthrough]];

    case -1:
        properties.GameLevel(0);
        properties.CityName("NoWhere");
		interfaceManager->dashboardWindow().cityName(properties.CityName());
        doPlayNewCity(properties, budget);
        break;

    case 0:
        throw std::runtime_error("Unexpected startup switch: " + std::to_string(startFlag));
        break;

    default: // scenario number
        doStartScenario(startFlag);
        break;
    }
}


void resetGame()
{
    simInit();
    primeGame(-1, cityProperties, budget);
}


void newGame()
{
    interfaceManager->fileIoDialog().clearSaveFilename();
    resetGame();
    drawBigMap();
}


void openGame()
{
    if (interfaceManager->fileIoDialog().pickOpenFile())
    {
        resetGame();
        LoadCity(interfaceManager->fileIoDialog().fullPath(), cityProperties, budget);
        interfaceManager->dashboardWindow().cityName(cityProperties.CityName());
        drawBigMap();
    }
}


void saveGame()
{
    if (!interfaceManager->fileIoDialog().filePicked() || SDL_GetModState() & SDL_KMOD_SHIFT)
    {
        if (!interfaceManager->fileIoDialog().pickSaveFile())
        {
            {
                return;
            }
        }
    }

    SaveCity(interfaceManager->fileIoDialog().fullPath(), cityProperties, budget);
}


void buildBigTileset()
{
    SDL_Surface* srcSurface = IMG_Load("images/tiles.xpm");
    SDL_Surface* dstSurface = SDL_CreateSurface(512, 512, SDL_PIXELFORMAT_RGBA32);

    SDL_Rect srcRect{ 0, 0, TileSize, TileSize };
    SDL_Rect dstRect{ 0, 0, TileSize, TileSize };

    for (int i = 0; i < TILE_COUNT; ++i)
    {
        srcRect.y = i * TileSize;
        dstRect = { (i % (TileSize * 2)) * TileSize, (i / (TileSize * 2)) * TileSize, TileSize, TileSize };
        SDL_BlitSurface(srcSurface, &srcRect, dstSurface, &dstRect);
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(MainWindowRenderer, dstSurface);

    SDL_DestroySurface(srcSurface);
    SDL_DestroySurface(dstSurface);

    if (!texture)
    {
        const std::string message(std::string("buildBigTileset(): ") + SDL_GetError());
        std::cout << message << std::endl;
        throw std::runtime_error(message);
    }

    BigTileset = buildTexture(texture);
}


void loadGraphics()
{
    buildBigTileset();
}


void loadFonts()
{
    //MainFont = std::make_unique<Font>("res/Raleway-Medium.ttf", 12);
    MainFont = std::make_unique<Font>("res/Raleway-Medium.ttf", 14);
}


void updateMapDrawParameters()
{
    FullMapViewRect =
    {
        static_cast<float>(MapViewOffset.x),
        static_cast<float>(MapViewOffset.y),
        static_cast<float>(WindowSize.x),
        static_cast<float>(WindowSize.y)
    };

    miniMapWindow->updateMapViewPosition(MapViewOffset);
}


void windowSize()
{
    SDL_GetWindowSize(MainWindow, &WindowSize.x, &WindowSize.y);;
}


void clampViewOffset()
{
    MapViewOffset =
    {
        std::clamp(MapViewOffset.x, 0, std::max(0, MainMapTexture.dimensions.x - WindowSize.x)),
        std::clamp(MapViewOffset.y, 0, std::max(0, MainMapTexture.dimensions.y - WindowSize.y))
    };
}


void minimapViewUpdated(const Point<int>& newOffset)
{
    MapViewOffset = newOffset.skewBy({ MiniMapTileMultiplier, MiniMapTileMultiplier });
    clampViewOffset();
    updateMapDrawParameters();
}


void centerWindow(WindowBase& window)
{
    window.position({ WindowSize.x / 2 - window.area().size.x / 2, WindowSize.y / 2 - window.area().size.y / 2 });
}


void windowResized(const Vector<int>& size)
{
    windowSize();
    clampViewOffset();

    miniMapWindow->updateViewportSize(WindowSize);

    updateMapDrawParameters();

	interfaceManager->centerWindows({ InterfaceManager::Window::Budget,
        InterfaceManager::Window::Evaluation,
        InterfaceManager::Window::Graph,
        InterfaceManager::Window::Options,
        InterfaceManager::Window::Query});

    interfaceManager->positionWindow(InterfaceManager::Window::Dashboard, DashboardWindowDefaultPosition);
    interfaceManager->positionWindow(InterfaceManager::Window::ToolPalette, ToolPaletteDefaultPosition);

    UiHeaderRect.w = static_cast<float>(WindowSize.x) - 20.0f;
}


void calculateMouseToWorld()
{
    const auto screenCell = positionToCell(EventHandling::MousePosition, MapViewOffset);
    
    TilePointedAt =
    {
       std::clamp(screenCell.x + (MapViewOffset.x / TileSize), 0, SimWidth - 1),
       std::clamp(screenCell.y + (MapViewOffset.y / TileSize), 0, SimHeight - 1)
    };

    TileHighlight =
    {
        (screenCell.x * TileSize) - MapViewOffset.x % TileSize,
        (screenCell.y * TileSize) - MapViewOffset.y % TileSize,
        TileSize, TileSize
    };

    miniMapWindow->updateTilePointedAt(TilePointedAt);
}


bool IgnoreToolMouseUp(Point<int>& mousePosition)
{
    if (interfaceManager->pointInWindow(EventHandling::MousePosition))
    {
        return true;
    }

    if (EventHandling::MouseDownPosition != EventHandling::MousePosition &&
        interfaceManager->pointInWindow(EventHandling::MouseDownPosition))
    {
        return true;
    }

    return false;
}


void SetSpeed(SimulationSpeed speed)
{
    if (paused())
    {
        resume();
    }

    simSpeed(speed);
}


void TogglePause()
{
    paused() ? resume() : pause();
}


void ToggleMiniMapVisibility()
{
    miniMapWindow->hidden() ? miniMapWindow->show() : miniMapWindow->hide();
}


void showEvaluationWindow()
{
    interfaceManager->evaluationWindow().setEvaluation(currentEvaluation());
    interfaceManager->showWindow(InterfaceManager::Window::Evaluation);
    currentEvaluationSeen();
}


void handleKeyEvent(SDL_Event& event)
{
    if (interfaceManager->optionsWindow().visible())
    {
        interfaceManager->optionsWindow().injectKeyDown(event.key.key);
        return;
    }

    switch (event.key.key)
    {
    case SDLK_ESCAPE:
        interfaceManager->hideAllWindows();
        interfaceManager->optionsWindow().setOptions(gameOptions);
        interfaceManager->optionsWindow().show();
        break;

    case SDLK_0:
    case SDLK_P:
    case SDLK_SPACE:
        TogglePause();
        break;

    case SDLK_1:
        SetSpeed(SimulationSpeed::Slow);
        break;

    case SDLK_2:
        SetSpeed(SimulationSpeed::Normal);
        break;

    case SDLK_3:
        SetSpeed(SimulationSpeed::Fast);
        break;

    case SDLK_4:
        SetSpeed(SimulationSpeed::AfricanSwallow);
        break;

    case SDLK_F2:
        saveGame();
        break;

    case SDLK_F3:
        openGame();
        break;

    case SDLK_F4:
        ToggleMiniMapVisibility();
        break;

    case SDLK_F5:
        //generateMonster();
        //generateTornado();
        //MakeFlood();
        //MakeMeltdown();
        //MakeFire();
        break;

    case SDLK_F7:
        newGame();
        break;

    case SDLK_F9:
        interfaceManager->showWindow(InterfaceManager::Window::Graph);
        break;

    case SDLK_F10:
		interfaceManager->showWindow(InterfaceManager::Window::Budget);
        break;

    case SDLK_F1:
        showEvaluationWindow();
        break;

    default:
        break;

    }
}


void handleMouseEvent(SDL_Event& event)
{
    if (event.window.windowID != MainWindowId) { return; }

    Vector<int> mouseMotionDelta{};
    Point<int> mousePosition = EventHandling::MousePosition;

    switch (event.type)
    {
    case SDL_EVENT_MOUSE_MOTION:
        EventHandling::MousePosition = { static_cast<int>(event.motion.x), static_cast<int>(event.motion.y) };
        mouseMotionDelta = { static_cast<int>(event.motion.xrel), static_cast<int>(event.motion.yrel) };

        DraggableToolVector = {};
        if (pendingToolProperties().draggable && EventHandling::MouseLeftDown && toolStart() != TilePointedAt)
        {
            DraggableToolVector = vectorFromPoints(toolStart(), TilePointedAt);
            validateDraggableToolVector(DraggableToolVector, budget);
        }

        calculateMouseToWorld();

		interfaceManager->injectMouseMotion(mouseMotionDelta);

        if ((SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_RMASK) != 0)
        {
            MapViewOffset -= mouseMotionDelta;
            clampViewOffset();
            updateMapDrawParameters();
            RightButtonDrag = true;
        }
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (event.button.button == SDL_BUTTON_LEFT)
        {
            EventHandling::MouseLeftDown = true;
            EventHandling::MouseDownPosition = { static_cast<int>(event.motion.x), static_cast<int>(event.motion.y) };

            if (interfaceManager->injectMouseDown(EventHandling::MousePosition))
            {
                return;
            }

            toolStart(TilePointedAt);
            
            if (!interfaceManager->budgetWindow().visible() && !pendingToolProperties().draggable)
            {
                ToolDown(TilePointedAt, budget);
            }

            if (pendingTool() == Tool::Query)
            {
                interfaceManager->queryWindow().setQueryResult(queryResult());
                interfaceManager->showWindow(InterfaceManager::Window::Query);
            }
        }
        break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event.button.button == SDL_BUTTON_LEFT)
        {
            EventHandling::MouseLeftDown = false;
            EventHandling::MouseClickPosition = { static_cast<int>(event.button.x), static_cast<int>(event.button.y) };
            interfaceManager->injectMouseUp();

            if (IgnoreToolMouseUp(mousePosition))
            {
                return;
            }

            toolEnd(TilePointedAt);
            
            if (pendingToolProperties().draggable)
            {
                executeDraggableTool(DraggableToolVector, TilePointedAt, budget);
            }
        }
        else if (event.button.button == SDL_BUTTON_RIGHT)
        {
            if (!RightButtonDrag)
            {
                pendingTool(Tool::None);
                interfaceManager->toolPalette().cancelTool();
            }

            RightButtonDrag = false;
        }
        break;

    default:
        break;
    }
}


void handleWindowEvent(SDL_Event& event)
{
    switch (event.window.type)
    {
    case SDL_EVENT_WINDOW_RESIZED:
        windowResized(Vector<int>{event.window.data1, event.window.data2});
        break;

    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        if (event.window.windowID == miniMapWindow->id())
        {
            miniMapWindow->hide();
        }

        if (event.window.windowID == MainWindowId)
        {
            simExit();
        }

        break;

    default:
        break;
    }
}


void pumpEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        miniMapWindow->injectEvent(event);

        if (event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST)
        {
            handleWindowEvent(event);
            continue;
        }

        switch (event.type)
        {
        case SDL_EVENT_KEY_DOWN:
            handleKeyEvent(event);
            break;

        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            handleMouseEvent(event);
            break;

        case SDL_EVENT_QUIT:
            simExit();
            break;
           
        default:
            break;
        }
    }
}


void initMainWindow()
{
	MainWindow = SDL_CreateWindow("Micropolis", 800, 600, SDL_WINDOW_RESIZABLE);
    if (!MainWindow)
    {
        throw std::runtime_error("initRenderer(): Unable to create primary window: " + std::string(SDL_GetError()));
    }
    
    SDL_SetWindowMinimumSize(MainWindow, 800, 600);
	SDL_SetWindowPosition(MainWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}


void initRenderer()
{
    initMainWindow();

    MainWindowRenderer = SDL_CreateRenderer(MainWindow, nullptr);
    
    if (!MainWindowRenderer)
    {
        throw std::runtime_error("initRenderer(): Unable to create renderer: " + std::string(SDL_GetError()));
    }

    SDL_SetRenderDrawBlendMode(MainWindowRenderer, SDL_BLENDMODE_BLEND);

    MainWindowId = SDL_GetWindowID(MainWindow);
}


void initViewParamters()
{
    windowSize();

    MainMapTexture.texture = SDL_CreateTexture(MainWindowRenderer, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_TARGET, SimWidth * 16, SimHeight * 16);
    MainMapTexture.dimensions = { SimWidth * 16, SimHeight * 16 };

    UiHeaderRect.w = static_cast<float>(WindowSize.x) - 20.0f;
    UiHeaderRect.h = 47.0f + 10.0f + static_cast<float>(MainFont->height()) + 10.0f; // 47 is height of rci indicator graphic
}


void drawTopUi()
{
    // Background
    SDL_SetRenderDrawColor(MainWindowRenderer, 0, 0, 0, 150);
    SDL_RenderFillRect(MainWindowRenderer, &UiHeaderRect);
    SDL_SetRenderDrawColor(MainWindowRenderer, 0, 0, 0, 255);
    SDL_RenderRect(MainWindowRenderer, &UiHeaderRect);

    stringRenderer->drawString(*MainFont, monthString(static_cast<Month>(lastCityMonth())), {static_cast<int>(UiHeaderRect.x) + 5, static_cast<int>(UiHeaderRect.y) + 5});
    stringRenderer->drawString(*MainFont, std::to_string(currentYear()), { static_cast<int>(UiHeaderRect.x) + 35, static_cast<int>(UiHeaderRect.y) + 5});

    stringRenderer->drawString(*MainFont, LastMessage(), { 100, static_cast<int>(UiHeaderRect.y) + 5 });

    const Point<int> budgetPosition{
        static_cast<int>(UiHeaderRect.x + UiHeaderRect.w - 5 - MainFont->width(currentBudget)),
        static_cast<int>(UiHeaderRect.y + 5)
    };
    
    stringRenderer->drawString(*MainFont, currentBudget, budgetPosition);
}


void DrawPendingTool(const ToolPalette& palette)
{
    if (palette.tool() == Tool::None || (pendingToolProperties().draggable && EventHandling::MouseLeftDown))
    {
        return;
    }


    const auto toolRect = fRectFromRect({
        TileHighlight.x - (pendingToolProperties().offset * TileSize),
        TileHighlight.y - (pendingToolProperties().offset * TileSize),
        pendingToolProperties().size * TileSize,
        pendingToolProperties().size * TileSize
        });

    if (palette.toolGost().texture)
    {
        SDL_RenderTexture(MainWindowRenderer, palette.toolGost().texture, &palette.toolGost().area, &toolRect);
        return;
    }

    SDL_SetRenderDrawColor(MainWindowRenderer, 255, 255, 255, 100);
    SDL_RenderFillRect(MainWindowRenderer, &toolRect);

    SDL_SetRenderDrawColor(MainWindowRenderer, 255, 255, 255, 255);
    SDL_RenderRect(MainWindowRenderer, &toolRect);
}


void drawDraggableToolVector()
{
    if (!EventHandling::MouseLeftDown) { return; }
    if (interfaceManager->pointInWindow(EventHandling::MouseDownPosition))
    {
        return;
    }
    
    auto toolRect = fRectFromRect({
        (toolStart().x * TileSize) - MapViewOffset.x,
        (toolStart().y * TileSize) - MapViewOffset.y,
        TileSize, TileSize
		});

    const int axis = longestAxis(DraggableToolVector);
    const int size = (std::abs(axis) * TileSize) + TileSize;

    const bool xAxisLarger = std::abs(DraggableToolVector.x) > std::abs(DraggableToolVector.y);
    xAxisLarger ? toolRect.w = static_cast<float>(size) : toolRect.h = static_cast<float>(size);

    if (axis < 0)
    {
        const int startValue = size - TileSize;
        xAxisLarger ? toolRect.x -= static_cast<float>(startValue) : toolRect.y -= static_cast<float>(startValue);
    }

    SDL_SetRenderDrawColor(MainWindowRenderer, 255, 255, 255, 100);
    SDL_RenderFillRect(MainWindowRenderer, &toolRect);

    SDL_SetRenderDrawColor(MainWindowRenderer, 255, 255, 255, 255);
    SDL_RenderRect(MainWindowRenderer, &toolRect);
}


void gameInit()
{
    simInit();

    primeGame(-1, cityProperties, budget);

    updateMapDrawParameters();
    initTimers();
}


void optionsChanged(const GameOptions& options)
{
	gameOptions = options;
}


void initUI()
{
    Point<int> mainWindowPosition{};
    SDL_GetWindowPosition(MainWindow, &mainWindowPosition.x, &mainWindowPosition.y);

    const auto mode = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());

    if(!mode)
    {
        throw std::runtime_error(std::string("initUI(): Unable to get desktop display mode: ") + SDL_GetError());
	}

    const Point<int> miniMapWindowPosition
    {
        std::clamp(mainWindowPosition.x - (SimWidth * MiniTileSize) - 10, 10, mode->w),
        std::clamp(mainWindowPosition.y, 10, mode->h)
    };

    miniMapWindow = std::make_unique<MiniMapWindow>(miniMapWindowPosition, Vector<int>{ SimWidth, SimHeight });
    miniMapWindow->updateViewportSize(WindowSize);
    miniMapWindow->focusOnMapCoordBind(&minimapViewUpdated);

    MiniMapWindow::EffectMapButtonMapping maps{
        { MiniMapWindow::ButtonId::Crime, CrimeMap},
        { MiniMapWindow::ButtonId::FireProtection, FireProtectionMap},
        { MiniMapWindow::ButtonId::LandValue, LandValueMap },
        { MiniMapWindow::ButtonId::PoliceProtection, PoliceProtectionMap },
        { MiniMapWindow::ButtonId::Pollution, PollutionMap },
        { MiniMapWindow::ButtonId::PopulationDensity, PopulationDensityMap },
        { MiniMapWindow::ButtonId::PopulationGrowth, RateOfGrowthMap },
        { MiniMapWindow::ButtonId::TrafficDensity, TrafficDensityMap }
    };

	miniMapWindow->linkEffectMaps(maps);

    stringRenderer = std::make_unique<StringRender>(MainWindowRenderer);

    interfaceManager = std::make_unique<InterfaceManager>(MainWindowRenderer, MainWindow, budget, currentRCI());

    interfaceManager->positionWindow(InterfaceManager::Window::Dashboard, DashboardWindowDefaultPosition);
    interfaceManager->positionWindow(InterfaceManager::Window::ToolPalette, ToolPaletteDefaultPosition);

    interfaceManager->optionsWindow().optionsChangedConnect(optionsChanged);
    interfaceManager->optionsWindow().newGameCallbackConnect(newGame);
    interfaceManager->optionsWindow().saveGameCallbackConnect(saveGame);
    interfaceManager->optionsWindow().openGameCallbackConnect(openGame);
}


void cleanUp()
{
    deinitTimers();

    miniMapWindow.reset(nullptr);
    
    interfaceManager.reset(nullptr);

    stringRenderer.reset(nullptr);

    MainFont.reset(nullptr);

    SDL_DestroyTexture(BigTileset.texture);

    SDL_DestroyRenderer(MainWindowRenderer);
    SDL_DestroyWindow(MainWindow);
}


void GameLoop()
{
    miniMapWindow->draw();
    drawBigMap();

    while (!Exit)
    {
        pumpEvents();

        SDL_RenderClear(MainWindowRenderer);
        SDL_RenderTexture(MainWindowRenderer, MainMapTexture.texture, &FullMapViewRect, nullptr);

        currentBudget = numberToDollarDecimal(budget.CurrentFunds());

        pendingTool(interfaceManager->toolPalette().tool());
        drawSprites();

        if (!interfaceManager->modalWindowVisible())
        {
            if (!interfaceManager->pointInWindow(EventHandling::MousePosition))
            {
                DrawPendingTool(interfaceManager->toolPalette());
                drawDraggableToolVector();
            }

            drawTopUi();

            if (currentEvaluation().needsAttention)
            {
                interfaceManager->evaluationWindow().setEvaluation(currentEvaluation());
                currentEvaluationSeen();
            }

            simLoop(SimulationStep);
        }

        interfaceManager->draw();

        SDL_RenderPresent(MainWindowRenderer);
        miniMapWindow->drawUI();

        newMap(false);
    }
}


#include <Windows.h>


int main(int argc, char* argv[])
{
    std::cout << "Starting Micropolis-SDL2 version " << MicropolisVersion << " originally by Will Wright and Don Hopkins." << std::endl;
    std::cout << "Original code Copyright (C) 2002 by Electronic Arts, Maxis. Released under the GPL v3" << std::endl;
    std::cout << "Modifications Copyright (C) 2022 - 2024 by Leeor Dicker. Available under the terms of the GPL v3" << std::endl << std::endl;
    
    std::cout << "Micropolis-SDL2 is not afiliated with Electronic Arts." << std::endl << std::endl;

    try
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            throw std::runtime_error(std::string("Unable to initialize SDL: ") + SDL_GetError());
        }

        initRenderer();
        loadGraphics();
        loadFonts();

        initViewParamters();
        initUI();

        gameInit();

        GameLoop();

        cleanUp();

        SDL_Quit();
    }
    catch(const std::exception& e)
    {
        std::string message(std::string(e.what()) + "\n\nMicropolis-SDL2PP will now close.");
        
        //#if defined(WIN32)
        MessageBoxA(nullptr, message.c_str(), "Micropolis-SDL2PP", MB_ICONERROR | MB_OK);
        //#else
        std::cout << message << std::endl;
        //#endif
    }

    return 0;
}
