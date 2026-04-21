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

#include "Math/Point.h"
#include "Math/Vector.h"

#include <vector>


/**
 * Naive abstraction of a 2D array using a 1D
 * array internally.
 */
class EffectMap
{
public:
    EffectMap() = delete;
    EffectMap(const Vector<int>& size) :
        mDimensions{ size },
        mSize{ size.x * size.y }
    {
        reset();
    }

    EffectMap& operator*(int scalar)
    {
        for (size_t i = 0; i < mEffectMap.size(); ++i)
        {
            mEffectMap[i] *= scalar;
        }

        return *this;
    }

    int value(const Point<int>& point) const
    {
        return mEffectMap[static_cast<size_t>((point.y * mDimensions.x) + point.x)];
    }

    int& value(const Point<int>& point)
    {
        return mEffectMap[static_cast<size_t>((point.y * mDimensions.x) + point.x)];
    }

    void reset()
    {
        mEffectMap.clear();
        fill(0);
    }

    const Vector<int> dimensions() const
    {
        return mDimensions;
    }

    void fill(const int value)
    {
        for (int i{ 0 }; i < mSize; ++i)
        {
            mEffectMap.push_back(value);
        }
    }

private:
    std::vector<int> mEffectMap{};

    Vector<int> mDimensions;
    int mSize{};
};
