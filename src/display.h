#pragma once

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

class Display
{
private:
	static void send_init_command(uint8_t commandByte, const uint8_t* dataBytes, uint8_t numDataBytes);
	static void send_command(uint8_t cmd);
	static void send_pixels(Color color, uint16_t len);
	static void send_hline(uint8_t x, uint8_t y, uint8_t len, Color color);
	static void send_vline(uint8_t x, uint8_t y, uint8_t len, Color color);
	static void set_addr_window(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

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
};
