#pragma once

#include "iopin.h"
#include "spimaster.h"

// The code below is lifted from Adafruit's LCD arduino library
// Thank you, Lady Ada!

// the pins and HW we use here
using mosi	= IoPin<'A', 4>;
using sck	= IoPin<'A', 6>;
using ss	= IoPin<'A', 7>;
using rst	= IoPin<'B', 0>;
using dc	= IoPin<'B', 1>;
using spi	= SpiMaster<0, 6>;

enum Color : uint16_t
{
	colBlack	= 0x0000,
	colWhite	= 0xFFFF,
	colRed		= 0xF800,
	colGreen	= 0x07E0,
	colBlue		= 0x001F,
	colCyan		= 0x07FF,
	colMagenta	= 0xF81F,
	colYellow	= 0xFFE0,
	colOrange	= 0xFC00,
};

struct GFXfont;

class Display
{
public:

	static void init();
	static void off();
	static void on();
	
	static void fill_screen(Color color);
	static void fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, Color color);
	static void draw_pixel(uint8_t x, uint8_t y, Color color);
	static void draw_circle(uint8_t x, uint8_t y, uint8_t r, Color color);
	static void fill_circle(uint8_t x, uint8_t y, uint8_t r, Color color);
	static void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, Color color);
	static void draw_hline(uint8_t x, uint8_t y, uint8_t len, Color color);
	static void draw_vline(uint8_t x, uint8_t y, uint8_t len, Color color);

	static void print(const char* str, bool smallFont, uint8_t x, uint8_t y, Color color, Color bgcolor);

	static void draw_raster(const uint8_t* raster, uint8_t x, uint8_t y, uint8_t w, uint8_t h, Color color, Color bgcolor);

private:
	// screen dimensions
	enum
	{
		WIDTH = 128,
		HEIGHT = 160,
	};

	static void send_init_command(uint8_t commandByte, const uint8_t* dataBytes, uint8_t numDataBytes);
	static void send_addr_window(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
	static void send_command(uint8_t cmd);
	static void send_pixels(Color color, uint16_t len);
	static void send_pixel(uint8_t x, uint8_t y, Color color);
	static void send_hline(uint8_t x, uint8_t y, uint8_t len, Color color);
	static void send_vline(uint8_t x, uint8_t y, uint8_t len, Color color);
	static void send_char(uint8_t x, uint8_t y, unsigned char c, Color color, Color bgcolor);
	static void send_char_custom(const GFXfont* gfxFont, uint8_t x, uint8_t y, unsigned char c, Color color);
};
