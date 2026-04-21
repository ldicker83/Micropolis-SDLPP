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

#include "RCI.h"

class Budget;
class CityProperties;

const RCI& currentRCI();

void SimFrame(CityProperties&, Budget&);
void condemnZone(int Xloc, int Yloc, int ch);
void DoSimInit(CityProperties&, Budget&);
void updateSpecialZones(bool powered, const CityProperties&);
void RepairZone(int ZCent, int zsize);
