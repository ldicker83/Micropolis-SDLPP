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
#include "ToolManager.h"
#include "GameDataLoader.h"


ToolManager::ToolManager() :
    mTools{ GameDataLoader::loadTools() },
    mCurrentTool{ nullptr }
{}


const Tool& ToolManager::tool(Tool::Type type) const
{
    return *std::find_if(mTools.begin(), mTools.end(), [type](const Tool& tool)
        {
            return tool.type == type;
        });
}


const Tool& ToolManager::currentTool() const
{
    return *mCurrentTool;
}


void ToolManager::currentTool(Tool::Type type)
{
    mCurrentTool = &tool(type);
}


void ToolManager::dragStart(const Point<int>& start)
{
    mDragStart = start;
}


const Point<int>& ToolManager::dragStart() const
{
    return mDragStart;
}


void ToolManager::dragEnd(const Point<int>& end)
{
    mDragEnd = end;
}


const Point<int>& ToolManager::dragEnd() const
{
    return mDragEnd;
}


const ZoneStats& ToolManager::queryResult() const
{
    return mQueryResult;
}


void ToolManager::queryResult(const ZoneStats& result)
{
    mQueryResult = result;
}
