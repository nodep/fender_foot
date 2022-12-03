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

enum ColorRGB : uint16_t
{
	rgbBlack	= 0x0000,
	rgbWhite	= 0xFFFF,
	rgbRed		= 0xF800,
	rgbGreen	= 0x07E0,
	rgbBlue		= 0x001F,
	rgbCyan		= 0x07FF,
	rgbMagenta	= 0xF81F,
	rgbYellow	= 0xFFE0,
	rgbOrange	= 0xFC00,
};

static ColorRGB Color2RGBMap[] = {
	rgbBlack,
	rgbWhite,
	rgbRed,
	rgbGreen,
	rgbBlue,
	rgbCyan,
	rgbMagenta,
	rgbYellow,
	rgbOrange,
};

inline ColorRGB color2rgb(const Color col)
{
	return Color2RGBMap[col];
}

template <Coord W, Coord H>
struct WindowRGB
{
	static const Coord Width = W;
	static const Coord Height = H;

	void start()	{}
	void finish()	{}

	ColorRGB	buffer[Width * Height];

	WindowRGB(ColorRGB colbgnd)
	{
		for (ColorRGB& pixel : buffer)
			pixel = colbgnd;
	}

	WindowRGB(Color colbgnd)
		: WindowRGB(color2rgb(colbgnd))
	{}

	void send_pixel(Coord x, Coord y, ColorRGB color)
	{
		buffer[y * Width + x] = color;
	}

	void send_pixel(Coord x, Coord y, Color color)
	{
		send_pixel(x, y, color2rgb(color));
	}

	void send_vline(Coord x, Coord y, Coord len, Color color)
	{
		for (Coord y1 = y; y1 < y + len; ++y1)
			send_pixel(x, y1, color);
	}
};

template <Coord W, Coord H>
struct Window
{
	static const Coord Width = W;
	static const Coord Height = H;

	void start()	{}
	void finish()	{}

	struct TwoPixels
	{
		unsigned int	first : 4;
		unsigned int	second : 4;

		TwoPixels()
		{}

		TwoPixels(Color col)
			: first(col), second(col)
		{}
	};
	
	TwoPixels buffer[(Width * Height + 1) / 2];

	Window(Color colbgnd)
	{
		for (TwoPixels& pixels : buffer)
			pixels = TwoPixels(colbgnd);
	}

	void send_vline(Coord x, Coord y, Coord len, Color color)
	{
		for (Coord y1 = y; y1 < y + len; ++y1)
			send_pixel(x, y1, color);
	}

	void send_pixel(Coord x, Coord y, Color color)
	{
		const size_t ndx = Width * y + x;
		if (ndx & 1)
			buffer[ndx / 2].second = color;
		else
			buffer[ndx / 2].first = color;
	}

	Color get_color(Coord x, Coord y)
	{
		const size_t ndx = Width * y + x;
		if (ndx & 1)
			return static_cast<Color>(buffer[ndx / 2].second);
		
		return static_cast<Color>(buffer[ndx / 2].first);
	}
};

struct GFXfont;

class Display
{
public:

	static void init();
	static void off();
	static void on();
	
	static void print(const char* str, bool smallFont, uint8_t x, uint8_t y, Color color, Color bgcolor);

	static void draw_raster(const uint8_t* raster, uint8_t x, uint8_t y, uint8_t w, uint8_t h, Color color, Color bgcolor);

	static void start()
	{
		ss::low();
	}

	static void finish()
	{
		ss::high();
	}

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
		start();
		set_addr_window(x, y, Win::Width, Win::Height);
		for (ColorRGB col : w.buffer)
			spi::send16(col);
		finish();
	}

	template <Coord WinWidth, Coord WinHeight>
	static void blit(Window<WinWidth, WinHeight>& w, Coord x, Coord y)
	{
		start();
		set_addr_window(x, y, WinWidth, WinHeight);
		for (Coord x = 0; x < WinHeight; x++)
			for (Coord y = 0; y < WinWidth; y++)
				send_color(w.get_color(x, y));
		finish();
	}

protected:

	template <typename T>
	friend void fill_rect(T& canvas, Coord x0, Coord y0, Coord w, Coord h, Color color);

	template <typename T>
	friend void draw_raster(T& d, const uint8_t* raster, Coord x, Coord y, Coord w, Coord h, Color color, Color bgcolor);

	// screen dimensions
	enum
	{
		WIDTH = 128,
		HEIGHT = 160,
	};

	static void send_init_command(uint8_t commandByte, const uint8_t* dataBytes, uint8_t numDataBytes);
	static void set_addr_window(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
	static void send_command(uint8_t cmd);

	static void send_char(uint8_t x, uint8_t y, unsigned char c, Color color, Color bgcolor);
	static void send_char_custom(const GFXfont* gfxFont, uint8_t x, uint8_t y, unsigned char c, Color color);
};

template <>
inline void fill_rect<Display>(Display&, Coord x, Coord y, Coord w, Coord h, Color color)
{
	Display::start();
	Display::set_addr_window(x, y, w, h);
	Display::send_colors(color, w * h);
	Display::finish();
}

template <>
inline void draw_raster<Display>(Display&, const uint8_t* raster, Coord x, Coord y, Coord w, Coord h, Color color, Color bgcolor)
{
	Display::start();

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

	Display::finish();
}