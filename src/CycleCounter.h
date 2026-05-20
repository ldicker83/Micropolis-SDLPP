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

namespace
{
    template<int N>
    concept PowerOfTwo = N > 0 && (N & (N - 1)) == 0;
}


template<int Limit> requires PowerOfTwo<Limit>
struct CycleCounter
{
    int advance() { return value = (value + 1) % Limit; }
    int current() const { return value; }
    void reset() { value = 0; }

private:
    int value{ 0 };
};
