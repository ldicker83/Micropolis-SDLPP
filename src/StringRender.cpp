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

#include "StringRender.h"

#include <algorithm>


StringRender::StringRender(SDL_Renderer* renderer) :
    mRenderer(renderer)
{}


void StringRender::drawString(Font& font, std::string_view text, const Point<int>& position) const
{
    if (text.empty()) { return; }

    const auto& gml = font.metrics();
    if (gml.empty()) { return; }

    int offset = 0;
    for (auto character : text)
    {
        const auto& gm = gml[std::clamp<std::size_t>((uint8_t)(character), 0, 255)];

        const auto glyphCellSize = font.glyphCellSize().to<float>();
        const auto adjustX = (gm.minX < 0) ? gm.minX : 0;

        const SDL_FRect srcRect
        {
            static_cast<float>(gm.uvRect.position.x),
            static_cast<float>(gm.uvRect.position.y),
            static_cast<float>(glyphCellSize.x),
            static_cast<float>(glyphCellSize.y)
        };

        const SDL_FRect dstRect
        {
            static_cast<float>(position.x + offset + adjustX),
            static_cast<float>(position.y),
            static_cast<float>(glyphCellSize.x),
            static_cast<float>(glyphCellSize.y)
        };

        SDL_RenderTexture(mRenderer, font.texture(), &srcRect, &dstRect);

        offset += gm.advance;
    }
}
