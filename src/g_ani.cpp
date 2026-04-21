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
#include "animtab.h"

#include "Map.h"

#include <iostream>

void animateTiles()
{
    int* tMapPtr = &(tileValue(0, 0));
    for (int i = SimWidth * SimHeight; i > 0; i--)
    {
        int tilevalue = (*tMapPtr);
        if (tilevalue & AnimatedBit)
        {
            const int tileflags = tilevalue & UpperMask;
            tilevalue &= LowerMask;

            tilevalue = aniTile[tilevalue];

            tilevalue |= tileflags;
            (*tMapPtr) = tilevalue;
        }
        tMapPtr++;
    }
}
