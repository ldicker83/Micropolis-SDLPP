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
#include "Texture.h"

#include "Graphics.h"

#if defined(__APPLE__)
#include <SDL3_Image/SDL_image.h>
#else
#include <SDL3_Image/SDL_image.h>
#endif

#include <iostream>
#include <sstream>

Texture loadTexture(SDL_Renderer* renderer, std::string_view filename)
{
    SDL_Surface* temp = IMG_Load(filename.data());
    if (!temp)
    {
		std::stringstream msg = std::stringstream{} << "loadTexture(): Unable to load '" << filename << "': " << SDL_GetError();
        std::cout << msg.str() << std::endl;
        throw std::runtime_error(msg.str());
    }

    SDL_Texture* out = SDL_CreateTextureFromSurface(renderer, temp);
    SDL_DestroySurface(temp);

    if (!out)
    {
		std::stringstream msg = std::stringstream{} << "loadTexture(): Unable to load '" << filename << "': " << SDL_GetError();
        std::cout << msg.str() << std::endl;
        throw std::runtime_error(msg.str());
    }

    return buildTexture(out);
}


Texture newTexture(SDL_Renderer* renderer, const Vector<int>& dimensions)
{

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, dimensions.x, dimensions.y);

    if (!texture)
    {
        std::cout << "newTexture(): Unable to create new texture: " << SDL_GetError() << std::endl;
        throw std::runtime_error(std::string{ "newTexture() : Unable to create new texture : " } + SDL_GetError());
    }

    return buildTexture(texture);
}


Texture buildTexture(SDL_Texture* texture)
{
    const auto textureProperties = SDL_GetTextureProperties(texture);
    const Vector<int> textureSize{
            static_cast<int>(SDL_GetNumberProperty(textureProperties, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0)),
            static_cast<int>(SDL_GetNumberProperty(textureProperties, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0))
    };

    SDL_DestroyProperties(textureProperties);

    const SDL_FRect area{
        0.0f, 0.0f,
        static_cast<float>(textureSize.x),
        static_cast<float>(textureSize.y)
    };

    return { texture, area, textureSize };
}


/**
 * Clears a texture and sets alpha to 0.
 */
void flushTexture(SDL_Renderer* renderer, Texture& texture)
{
    SDL_SetRenderTarget(renderer, texture.texture);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_RenderFillRect(renderer, &texture.area);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}
