#include <stdint.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "graph.h"
#include "graphtext.h"

// default font
#include "fonts/glcdfont.h"

// custom fonts
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSansBold9pt7b.h"

#include "fonts/FreeSerif9pt7b.h"
#include "fonts/FreeSerifBold9pt7b.h"

#include "fonts/FreeMono9pt7b.h"
#include "fonts/FreeMonoBold9pt7b.h"

#include "fonts/Org_01.h"

const GFXfont* largeFont = &FreeSans9pt7b;

uint16_t get_text_width_large(const char* text)
{
	const uint8_t first = pgm_read_byte(&largeFont->first);
	uint16_t result = 0;
	while (*text)
	{
		const uint8_t c = *text;

		if (c >= first)
		{
			const GFXglyph* glyph = pgm_read_glyph_ptr(largeFont, c - first);
			result += pgm_read_byte(&glyph->xAdvance);
		}

		++text;
	}

	return result;
}
