#pragma once

#include "iopin.h"
#include "spimaster.h"

#include "graph.h"

// The code below is lifted from Adafruit's LCD arduino library
// Thank you, Lady Ada!

// the pins and HW we use here
using mosi	= IoPin<'A', 4>;
using sck	= IoPin<'A', 6>;
using ss	= IoPin<'A', 7>;
using rst	= IoPin<'B', 0>;
using dc	= IoPin<'B', 1>;
using spi	= SpiMaster<0, 6>;

struct GFXfont;

class Display
{
public:

	struct Transaction
	{
		Transaction()
		{
			ss::low();
		}

		~Transaction()
		{
			ss::high();
		}
	};

	static const Coord Width = 128;
	static const Coord Height = 160;

	static void init();
	static void off();
	static void on();
	
	static void print(const char* str, bool smallFont, uint8_t x, uint8_t y, Color color, Color bgcolor);

	static void send_pixel(uint8_t x, uint8_t y, Color color)
	{
		set_addr_window(x, y, 1, 1);
		send_color(color);
	}

	static void send_color(Color color)
	{
		spi::send16(color2rgb(color));
	}

	static void send_colors(Color color, uint16_t len)
	{
		for (uint16_t c = 0; c < len; c++)
			send_color(color);
	}

	static void send_hline(uint8_t x, uint8_t y, uint8_t len, Color color)
	{
		set_addr_window(x, y, len, 1);
		send_colors(color, len);
	}

	static void send_vline(uint8_t x, uint8_t y, uint8_t len, Color color)
	{
		set_addr_window(x, y, 1, len);
		send_colors(color, len);
	}

	template <typename Win>
	static void blit(Win& w, Coord x, Coord y)
	{
		typename Display::Transaction t;

		set_addr_window(x, y, Win::Width, Win::Height);
		for (ColorRGB col : w.buffer)
			spi::send16(col);
	}

	template <Coord WinWidth, Coord WinHeight>
	static void blit(Window<WinWidth, WinHeight>& w, Coord x, Coord y)
	{
		typename Display::Transaction t;

		set_addr_window(x, y, WinWidth, WinHeight);
		for (Coord x = 0; x < WinHeight; x++)
			for (Coord y = 0; y < WinWidth; y++)
				send_color(w.get_color(x, y));
	}

protected:

	friend void fill_rect<Display>(Display& d, Coord x0, Coord y0, Coord w, Coord h, Color color);
	friend void draw_raster<Display>(Display& d, const uint8_t* raster, Coord x, Coord y, Coord w, Coord h, Color color, Color bgcolor);

	static void send_init_command(uint8_t commandByte, const uint8_t* dataBytes, uint8_t numDataBytes);
	static void set_addr_window(Coord x, Coord y, Coord w,  Coord h);
	static void send_command(uint8_t cmd);

	static void send_char(uint8_t x, uint8_t y, unsigned char c, Color color, Color bgcolor);
	static void send_char_custom(const GFXfont* gfxFont, Coord x, Coord y, unsigned char c, Color color);
};

template <>
inline void fill_rect<Display>(Display&, Coord x, Coord y, Coord w, Coord h, Color color)
{
	typename Display::Transaction t;

	Display::set_addr_window(x, y, w, h);
	Display::send_colors(color, w * h);
}

template <>
inline void draw_raster<Display>(Display&, const uint8_t* raster, Coord x, Coord y, Coord w, Coord h, Color color, Color bgcolor)
{
	typename Display::Transaction t;

	Display::set_addr_window(x, y, w, h);

	Color curr_color = bgcolor;
	while (true)
	{
		const uint8_t num_pixels = pgm_read_byte(raster++);

		if (num_pixels == 0)
			break;

		Display::send_colors(curr_color, num_pixels);

		if (curr_color == color)
			curr_color = bgcolor;
		else
			curr_color = color;
	}
}
