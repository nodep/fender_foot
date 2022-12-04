#pragma once

#include <stdint.h>

#include <avr/pgmspace.h>

#include "graph.h"

// Font data stored PER GLYPH
struct GFXglyph
{
	uint16_t	bitmapOffset;	// Pointer into GFXfont->bitmap
	uint8_t		width;			// Bitmap dimensions in pixels
	uint8_t		height;			// Bitmap dimensions in pixels
	uint8_t		xAdvance;		// Distance to advance cursor (x axis)
	int8_t		xOffset;		// X dist from cursor pos to UL corner
	int8_t		yOffset;		// Y dist from cursor pos to UL corner
};

// Data stored for FONT AS A WHOLE
struct GFXfont
{
	uint8_t*	bitmap;		// Glyph bitmaps, concatenated
	GFXglyph*	glyph;		// Glyph array
	uint16_t	first;		// ASCII extents (first char)
	uint16_t	last;		// ASCII extents (last char)
	uint8_t		yAdvance;	// Newline distance (y axis)
};

#define pgm_read_pointer(addr) ((void*) pgm_read_dword(addr))

inline GFXglyph* pgm_read_glyph_ptr(const GFXfont* gfxFont, const uint8_t c)
{
	return &static_cast<GFXglyph*>(pgm_read_pointer(&gfxFont->glyph))[c];
}

inline uint8_t* pgm_read_bitmap_ptr(const GFXfont* gfxFont)
{
	return static_cast<uint8_t*>(pgm_read_pointer(&gfxFont->bitmap));
}

// default font
#include "fonts/glcdfont.h"

template <typename Canvas, typename ColorT>
void print_char(Canvas& canvas, const Coord x, const Coord y, const unsigned char c, const ColorT color, const ColorT bgcolor)
{
	if (x >= Canvas::Width		// Clip right
		|| y >= Canvas::Height	// Clip bottom
		|| x + 6 - 1 < 0		// Clip left
		|| y + 8 - 1 < 0)		// Clip top
		return;

	// char bitmap = 5 columns
	for (int8_t i = 0; i < 5; i++)
	{
		Coord line = pgm_read_byte(&font[c * 5 + i]);
		for (Coord j = 0; j < 8; j++, line >>= 1)
		{
			if (line & 1)
				canvas.pixel(x + i, y + j, color);
			else if (bgcolor != color)
				canvas.pixel(x + i, y + j, bgcolor);
		}
	}

	// if opaque, draw vertical line for last column
	if (bgcolor != color)
		canvas.vline(x + 5, y, 8, bgcolor);
}

template <typename Canvas, typename ColorT>
void print_char_custom(Canvas& canvas, const GFXfont* gfxFont, const Coord x, const Coord y, unsigned char c, const ColorT color)
{
	c -= pgm_read_byte(&gfxFont->first);

	const GFXglyph* glyph = pgm_read_glyph_ptr(gfxFont, c);
	const uint8_t* bitmap = pgm_read_bitmap_ptr(gfxFont);

	uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
	const uint8_t w = pgm_read_byte(&glyph->width);
	const uint8_t h = pgm_read_byte(&glyph->height);
	const int8_t xo = pgm_read_byte(&glyph->xOffset);
	const int8_t yo = pgm_read_byte(&glyph->yOffset);
	uint8_t bits = 0, bit = 0;

	for (Coord yy = 0; yy < h; yy++)
	{
		for (Coord xx = 0; xx < w; xx++)
		{
			if ((bit++ & 7) == 0)
				bits = pgm_read_byte(&bitmap[bo++]);

			if (bits & 0x80)
				canvas.pixel(x + xo + xx, y + yo + yy, color);

			bits <<= 1;
		}
	}
}

// custom fonts
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSansBold9pt7b.h"

#include "fonts/FreeSerif9pt7b.h"
#include "fonts/FreeSerifBold9pt7b.h"

#include "fonts/FreeMono9pt7b.h"
#include "fonts/FreeMonoBold9pt7b.h"

#include "fonts/Org_01.h"

const GFXfont* largeFont = &FreeSans9pt7b;

template <typename Canvas, typename ColorT>
void print(Canvas& canvas, const char* str, const bool smallFont, const Coord x, const Coord y, const Color color, const ColorT bgcolor)
{
	typename Canvas::Transaction t;

	uint8_t cursor_x = x;
	uint8_t cursor_y = y;

	const bool wrap = true;

	const GFXfont* gfxFont = nullptr;
	if (!smallFont)
	{
		gfxFont = largeFont;
		cursor_y += 14;
	}
	
	while (*str)
	{
		const char c = *str++;

		if (!gfxFont)
		{
			if (c == '\n')
			{
				cursor_x = x;
				cursor_y += 8;
			}
			else if (c != '\r')
			{
				if (wrap  &&  cursor_x + 6 > Canvas::Width)
				{
					cursor_x = x;
					cursor_y += 8;
				}
				print_char(canvas, cursor_x, cursor_y, c, color, bgcolor);
				cursor_x += 6;
			}
		}
		else
		{
			if (c == '\n')
			{
				cursor_x = x;
				cursor_y += pgm_read_byte(&gfxFont->yAdvance);
			}
			else if (c != '\r')
			{
				const uint8_t first = pgm_read_byte(&gfxFont->first);
				if (c >= first  &&  c <= pgm_read_byte(&gfxFont->last))
				{
					const GFXglyph* glyph = pgm_read_glyph_ptr(gfxFont, c - first);
					const uint8_t w = pgm_read_byte(&glyph->width);
					const uint8_t h = pgm_read_byte(&glyph->height);

					if (w > 0  &&  h > 0)
					{
						const int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset);
						if (wrap  &&  cursor_x + xo + w > Canvas::Width)
						{
							cursor_x = 0;
							cursor_y += pgm_read_byte(&gfxFont->yAdvance);
						}
						print_char_custom(canvas, gfxFont, cursor_x, cursor_y, c, color);
					}

					cursor_x += pgm_read_byte(&glyph->xAdvance);
				}
			}
		}
	}
}
