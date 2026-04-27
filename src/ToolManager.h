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

#include "Tool.h"


class ToolManager
{
public:

};


const Tool& tool(Tool::Type);

const ZoneStats& queryResult();

const Tool& pendingTool();
void pendingTool(const Tool::Type);

void toolStart(const Point<int>&);
const Point<int>& toolStart();

void toolEnd(const Point<int>&);
const Point<int>& toolEnd();

void ToolDown(const Point<int> location, Budget& budget);

int longestAxis(const Vector<int>&);
void validateDraggableToolVector(Vector<int>&, Budget&);
void executeDraggableTool(const Vector<int>&, const Point<int>&, Budget&);
