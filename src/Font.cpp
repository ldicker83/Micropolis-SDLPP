// ==================================================================================
// = NAS2D
// = Copyright © 2008 - 2026 Leeor Dicker
// ==================================================================================
// = NAS2D is distributed under the terms of the zlib license. You are free to copy,
// = modify and distribute the software under the terms of the zlib license.
// =
// = Acknowledgement of your use of NAS2D is appriciated but is not required.
// ==================================================================================
#include "Font.h"

#include "Math/PointInRectangleRange.h"

#if defined(__APPLE__)
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf//SDL_ttf.h>
#else
#include <SDL3_Image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#endif

#include <cmath>
#include <algorithm>
#include <cstddef>
#include <stdexcept>

extern SDL_Renderer* MainWindowRenderer;


namespace {
	struct ColorMasks
	{
		unsigned int red;
		unsigned int green;
		unsigned int blue;
		unsigned int alpha;
	};

	constexpr ColorMasks MasksLittleEndian{0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000};
	constexpr ColorMasks MasksBigEndian{0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff};
	constexpr ColorMasks MasksDefault = (SDL_BYTEORDER == SDL_LIL_ENDIAN) ? MasksLittleEndian : MasksBigEndian;

	const int ASCII_TABLE_COUNT = 256;
	const int GLYPH_MATRIX_SIZE = 16;
	const int BITS_32 = 32;

	Font::FontInfo load(const std::string& path, unsigned int ptSize);
	SDL_Texture* generateFontTexture(SDL_Surface* fontSurface, std::vector<Font::GlyphMetrics>& glyphMetricsList, Vector<int> glyphSize);
	SDL_Surface* generateFontSurface(TTF_Font* font, Vector<int> characterSize);
	Vector<int> maxCharacterDimensions(TTF_Font* font);
	Vector<int> roundedCharacterDimensions(Vector<int> maxSize);
	void fillInCharacterDimensions(TTF_Font* font, std::vector<Font::GlyphMetrics>& glyphMetricsList);
	void fillInTextureCoordinates(std::vector<Font::GlyphMetrics>& glyphMetricsList, Vector<int> glyphSize);
}


/**
 * Instantiate a Font using a TrueType or OpenType font.
 *
 * \param	filePath	Path to a font file.
 * \param	ptSize		Point size of the font. Defaults to 12pt.
 */
Font::Font(const std::string& filePath, unsigned int ptSize) :
	mResourceName{filePath + "_" + std::to_string(ptSize) + "pt"},
	mFontInfo{load(filePath, ptSize)}
{
}


Font::~Font()
{
	if (mFontInfo.texture)
	{
		SDL_DestroyTexture(mFontInfo.texture);
		mFontInfo.texture = nullptr;
	}
}


Vector<int> Font::glyphCellSize() const
{
	return mFontInfo.glyphSize;
}


Vector<int> Font::size(std::string_view string) const
{
	const auto& gml = mFontInfo.metrics;
	if (gml.empty()) { return { 0, 0 }; }

	Vector<int> size{ 0, 0 };
	int lineWidth = 0;
	for (auto character : string)
	{
		if (character == '\n')
		{
			size.y += height();
			size.x = std::max(size.x, lineWidth);
			lineWidth = 0;
		}
		else
		{
			auto glyph = std::clamp<std::size_t>(static_cast<uint8_t>(character), 0, 255);
			lineWidth += gml[glyph].advance;
		}
	}
	if (!string.empty())
	{
		size.y += height();
		size.x = std::max(size.x, lineWidth);
	}

	return size;
}


/**
 * Gets the width in pixels of a string rendered using the Font.
 *
 * \param	string		String to get the width of.
 */
int Font::width(std::string_view string) const
{
	return size(string).x;
}


/**
 * Gets the height in pixels of the Font.
 */
int Font::height() const
{
	return mFontInfo.height;
}


/**
 * The maximum pixel ascent of all glyphs in the Font.
 */
int Font::ascent() const
{
	return mFontInfo.ascent;
}


/**
 * Returns the point size of the Font.
 */
unsigned int Font::ptSize() const
{
	return mFontInfo.pointSize;
}


const std::vector<Font::GlyphMetrics>& Font::metrics() const
{
	return mFontInfo.metrics;
}


namespace {
	/**
	 * Loads a TrueType or OpenType font from a file.
	 *
	 * \param	path	Path to the TTF or OTF font file.
	 * \param	ptSize	Point size to use when loading the font.
	 */
	Font::FontInfo load(const std::string& path, unsigned int ptSize)
	{
		std::string fontname = path + "_" + std::to_string(ptSize) + "pt";

		if (TTF_WasInit() == 0)
		{
			if (!TTF_Init())
			{
				throw std::runtime_error("Unable to load font: " + std::string{ SDL_GetError() });
			}
		}

		TTF_Font* font = TTF_OpenFont(path.c_str(), static_cast<float>(ptSize));
		if (!font)
		{
			throw std::runtime_error("Unable to load font: " + std::string{ SDL_GetError() });
		}

		Font::FontInfo fontInfo;
		auto& glm = fontInfo.metrics;
		fillInCharacterDimensions(font, glm);
		const auto charBoundsSize = maxCharacterDimensions(font);
		const auto roundedCharSize = roundedCharacterDimensions(charBoundsSize);
		SDL_Surface* fontSurface = generateFontSurface(font, roundedCharSize);

		fontInfo.pointSize = ptSize;
		fontInfo.height = TTF_GetFontHeight(font);
		fontInfo.ascent = TTF_GetFontAscent(font);
		fontInfo.glyphSize = roundedCharSize;
		fontInfo.texture = generateFontTexture(fontSurface, glm, roundedCharSize);
		SDL_DestroySurface(fontSurface);
		TTF_CloseFont(font);

		return fontInfo;
	}

	/**
	 * Generates a glyph map of all ASCII standard characters from 0 - 255.
	 *
	 * Internal function used to generate a glyph texture map from an TTF_Font struct.
	 */
	SDL_Texture* generateFontTexture(SDL_Surface* fontSurface, std::vector<Font::GlyphMetrics>& glyphMetricsList, Vector<int> glyphSize)
	{
		fillInTextureCoordinates(glyphMetricsList, glyphSize);

		SDL_Texture* out = SDL_CreateTextureFromSurface(MainWindowRenderer, fontSurface);

		if (!out)
		{
			throw std::runtime_error(std::string("Unable to create font texture: ") + SDL_GetError());
		}
		
		return out;
	}


	SDL_Surface* generateFontSurface(TTF_Font* font, Vector<int> characterSize)
	{
		const auto matrixSize = characterSize * GLYPH_MATRIX_SIZE;
		auto fontSurface = SDL_CreateSurface(matrixSize.x, matrixSize.y, SDL_PIXELFORMAT_RGBA32);

		SDL_Color white = { 255, 255, 255, 255 };
		for (const auto glyphPosition : PointInRectangleRange(Rectangle{0, 0, GLYPH_MATRIX_SIZE, GLYPH_MATRIX_SIZE}))
		{
			const std::size_t glyph = static_cast<std::size_t>(glyphPosition.y) * GLYPH_MATRIX_SIZE + glyphPosition.x;

			// Avoid glyph 0, which has size 0 for some fonts
			// SDL_TTF will produce errors for a glyph of size 0
			if (glyph == 0) { continue; }

			SDL_Surface* characterSurface = TTF_RenderGlyph_Blended(font, static_cast<uint16_t>(glyph), white);
			if (!characterSurface)
			{
				continue;
			}

			SDL_SetSurfaceBlendMode(characterSurface, SDL_BLENDMODE_NONE);
			const auto pixelPosition = glyphPosition.skewBy(characterSize);
			SDL_Rect rect = { pixelPosition.x, pixelPosition.y, 0, 0 };
			SDL_BlitSurface(characterSurface, nullptr, fontSurface, &rect);
			SDL_DestroySurface(characterSurface);
		}
		return fontSurface;
	}


	Vector<int> maxCharacterDimensions(TTF_Font* font)
	{
		Vector<int> size{0, 0};

		for (int i = 0; i < 256; ++i)
		{
			Vector<int> sizeChar;
			char text[2] = {static_cast<char>(i), 0};
			TTF_GetStringSize(font, text, 0, &sizeChar.x, &sizeChar.y);
			size = {std::max({size.x, sizeChar.x}), std::max({size.y, sizeChar.y})};
		}
		return size;
	}


	uint32_t roundUpPowerOf2(uint32_t number)
	{
		--number;
		number |= number >> 1;
		number |= number >> 2;
		number |= number >> 4;
		number |= number >> 8;
		number |= number >> 16;
		return ++number;
	}


	Vector<int> roundedCharacterDimensions(Vector<int> maxSize)
	{
		const auto maxSizeUint32 = maxSize.to<uint32_t>();
		const auto roundedUpSize = Vector{roundUpPowerOf2(maxSizeUint32.x), roundUpPowerOf2(maxSizeUint32.y)};
		return roundedUpSize.to<int>();
	}


	void fillInCharacterDimensions(TTF_Font* font, std::vector<Font::GlyphMetrics>& glyphMetricsList)
	{
		for (Uint16 i = 0; i < ASCII_TABLE_COUNT; i++)
		{
			auto& metrics = glyphMetricsList.emplace_back();
			TTF_GetGlyphMetrics(font, i, &metrics.minX, &metrics.maxX, &metrics.minY, &metrics.maxY, &metrics.advance);
		}
	}


	void fillInTextureCoordinates(std::vector<Font::GlyphMetrics>& glyphMetricsList, Vector<int> glyphSize)
	{
		for (const auto glyphPosition : PointInRectangleRange(Rectangle{0, 0, GLYPH_MATRIX_SIZE, GLYPH_MATRIX_SIZE}))
		{
			const std::size_t glyph = static_cast<std::size_t>(glyphPosition.y) * GLYPH_MATRIX_SIZE + glyphPosition.x;
			const auto uvStart = glyphPosition.skewBy(glyphSize);

			auto& uvRect = glyphMetricsList[glyph].uvRect;
			uvRect = { uvStart.x, uvStart.y, glyphSize.x, glyphSize.y };
		}
	}
}
