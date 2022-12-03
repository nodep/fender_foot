#include <stdint.h>
#include <stdlib.h>

#include <avr/pgmspace.h>
#include <util/delay.h>

#include "iopin.h"
#include "spimaster.h"

#include "display.h"

enum ST77Constants : uint8_t
{
	ST77XX_NOP = 0x00,
	ST77XX_SWRESET = 0x01,
	ST77XX_RDDID = 0x04,
	ST77XX_RDDST = 0x09,

	ST77XX_SLPIN = 0x10,
	ST77XX_SLPOUT = 0x11,
	ST77XX_PTLON = 0x12,
	ST77XX_NORON = 0x13,

	ST77XX_INVOFF = 0x20,
	ST77XX_INVON = 0x21,
	ST77XX_DISPOFF = 0x28,
	ST77XX_DISPON = 0x29,
	ST77XX_CASET = 0x2A,
	ST77XX_RASET = 0x2B,
	ST77XX_RAMWR = 0x2C,
	ST77XX_RAMRD = 0x2E,

	ST77XX_PTLAR = 0x30,
	ST77XX_TEOFF = 0x34,
	ST77XX_TEON = 0x35,
	ST77XX_MADCTL = 0x36,
	ST77XX_COLMOD = 0x3A,

	ST77XX_MADCTL_MY = 0x80,
	ST77XX_MADCTL_MX = 0x40,
	ST77XX_MADCTL_MV = 0x20,
	ST77XX_MADCTL_ML = 0x10,
	ST77XX_MADCTL_RGB = 0x00,

	ST77XX_RDID1 = 0xDA,
	ST77XX_RDID2 = 0xDB,
	ST77XX_RDID3 = 0xDC,
	ST77XX_RDID4 = 0xDD,

	// Some register settings
	ST7735_MADCTL_BGR = 0x08,
	ST7735_MADCTL_MH = 0x04,

	ST7735_FRMCTR1 = 0xB1,
	ST7735_FRMCTR2 = 0xB2,
	ST7735_FRMCTR3 = 0xB3,
	ST7735_INVCTR = 0xB4,
	ST7735_DISSET5 = 0xB6,

	ST7735_PWCTR1 = 0xC0,
	ST7735_PWCTR2 = 0xC1,
	ST7735_PWCTR3 = 0xC2,
	ST7735_PWCTR4 = 0xC3,
	ST7735_PWCTR5 = 0xC4,
	ST7735_VMCTR1 = 0xC5,

	ST7735_PWCTR6 = 0xFC,

	ST7735_GMCTRP1 = 0xE0,
	ST7735_GMCTRN1 = 0xE1,

	ST_CMD_DELAY = 0x80,		// special signifier for = command lists
};

static const uint8_t PROGMEM initCommands[] =
{										// 7735R init
	ST77XX_SWRESET, ST_CMD_DELAY,		// Software reset, 0 args, w/delay
		150,							// 150 ms delay
	ST77XX_SLPOUT, ST_CMD_DELAY,		// Out of sleep mode, 0 args, w/delay
		255,							// 500 ms delay
	ST7735_FRMCTR1, 3,					// Framerate ctrl - normal mode, 3 arg:
		0x01, 0x2C, 0x2D,				// Rate = fosc/(1x2+40) * (LINE+2C+2D)
	ST7735_FRMCTR2, 3,					// Framerate ctrl - idle mode, 3 args:
		0x01, 0x2C, 0x2D,				// Rate = fosc/(1x2+40) * (LINE+2C+2D)
	ST7735_FRMCTR3, 6,					// Framerate - partial mode, 6 args:
		0x01, 0x2C, 0x2D,				// Dot inversion mode
		0x01, 0x2C, 0x2D,				// Line inversion mode
	ST7735_INVCTR, 1,					// Display inversion ctrl, 1 arg:
		0x07,							// No inversion
	ST7735_PWCTR1, 3,					// Power control, 3 args, no delay:
		0xA2,
		0x02,							// -4.6V
		0x84,							// AUTO mode
	ST7735_PWCTR2, 1,					// Power control, 1 arg, no delay:
		0xC5,							// VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
	ST7735_PWCTR3, 2,					// Power control, 2 args, no delay:
		0x0A,							// Opamp current small
		0x00,							// Boost frequency
	ST7735_PWCTR4, 2,					// Power control, 2 args, no delay:
		0x8A,							// BCLK/2,
		0x2A,							// opamp current small & medium low
	ST7735_PWCTR5, 2,					// Power control, 2 args, no delay:
		0x8A, 0xEE,
	ST7735_VMCTR1, 1,					// Power control, 1 arg, no delay:
		0x0E,
	ST77XX_INVOFF, 0,					// Don't invert display, no args
	ST77XX_MADCTL, 1,					// Mem access ctl (directions), 1 arg:
		0xC8,							// row/col addr, bottom-top refresh
	ST77XX_COLMOD, 1,					// set color mode, 1 arg, no delay:
		0x05,							// 16-bit color

	ST77XX_CASET, 4,					// Column addr set, 4 args, no delay:
		0x00, 0x00,						// XSTART = 0
		0x00, 0x7F,						// XEND = 127
	ST77XX_RASET, 4,					// Row addr set, 4 args, no delay:
		0x00, 0x00,						// XSTART = 0
		0x00, 0x9F,						// XEND = 159

	ST7735_GMCTRP1, 16,					// Gamma Adjustments (pos. polarity), 16 args + delay:
		0x02, 0x1c, 0x07, 0x12,			// (Not entirely necessary, but provides
		0x37, 0x32, 0x29, 0x2d,			//  accurate colors)
		0x29, 0x25, 0x2B, 0x39,
		0x00, 0x01, 0x03, 0x10,
	ST7735_GMCTRN1, 16,					// Gamma Adjustments (neg. polarity), 16 args + delay:
		0x03, 0x1d, 0x07, 0x06,			// (Not entirely necessary, but provides
		0x2E, 0x2C, 0x29, 0x2D,			//  accurate colors)
		0x2E, 0x2E, 0x37, 0x3F,
		0x00, 0x00, 0x02, 0x10,
	ST77XX_NORON, ST_CMD_DELAY,			// Normal display on, no args, w/delay
		10,								// 10 ms delay
	ST77XX_DISPON, ST_CMD_DELAY,		// Main screen turn on, no args w/delay
		100,							// 100 ms delay

	ST77XX_MADCTL, 1,					// change MADCTL color filter
		0xC0,

	0x00,								// EOF
};

void Display::init()
{
	mosi::dir_out();
	sck::dir_out();

	rst::low();
	rst::dir_out();

	ss::high();
	ss::dir_out();

	dc::high();
	dc::dir_out();

	_delay_ms(100);
	rst::high();
	_delay_ms(200);

	spi::init();

	const uint8_t* addr = initCommands;
	uint8_t cmd;

	while ((cmd = pgm_read_byte(addr++)) != 0)
	{
		// number of args to follow
		uint8_t numArgs = pgm_read_byte(addr++);

		// if the hibit is set, delay time follows args
		uint16_t ms = static_cast<uint8_t>(numArgs & ST_CMD_DELAY);

		// mask out delay bit
		numArgs &= ~ST_CMD_DELAY;
		send_init_command(cmd, addr, numArgs);
		addr += numArgs;

		if (ms)
		{
			// Read post-command delay time (ms)
			ms = pgm_read_byte(addr++);
			if (ms == 255)
				ms = 500;
			while (ms--)
				_delay_ms(1);
		}
	}
}

void Display::on()
{
	ss::low();
	send_command(ST77XX_DISPON);
	ss::high();
}

void Display::off()
{
	ss::low();
	send_command(ST77XX_DISPOFF);
	ss::high();
}

void Display::send_command(const uint8_t cmd)
{
	dc::low();
	spi::send(cmd);
	dc::high();
}

void Display::send_init_command(const uint8_t commandByte, const uint8_t* dataBytes, const uint8_t numDataBytes)
{
	ss::low();

	send_command(commandByte);

	for (int i = 0; i < numDataBytes; i++)
		spi::send(pgm_read_byte(dataBytes++));

	ss::high();
}

void Display::send_addr_window(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h)
{
	const uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
	const uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);

	send_command(ST77XX_CASET); // column addr set
	spi::send32(xa);

	send_command(ST77XX_RASET); // ryow addr set
	spi::send32(ya);

	send_command(ST77XX_RAMWR); // write to RAM
}

void Display::send_pixels(const Color color, const uint16_t len)
{
	for (uint16_t c = 0; c < len; c++)
		spi::send16(color2rgb(color));
}

void Display::fill_screen(const Color color)
{
	fill_rect(0, 0, WIDTH, HEIGHT, color);
}

void Display::fill_rect(const uint8_t x, const uint8_t y, const uint8_t w, const uint8_t h, const Color color)
{
	ss::low();
	send_addr_window(x, y, w, h);
	send_pixels(color, w * h);
	ss::high();
}

void Display::send_pixel(const uint8_t x, const uint8_t y, const Color color)
{
	send_addr_window(x, y, 1, 1);
	spi::send16(color2rgb(color));
}

void Display::draw_pixel(const uint8_t x, const uint8_t y, const Color color)
{
	ss::low();
	send_pixel(x, y, color);
	ss::high();
}

void Display::draw_circle(const uint8_t x0, const uint8_t y0, const uint8_t r, const Color color)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	start();

	send_pixel(x0, y0 + r, color);
	send_pixel(x0, y0 - r, color);
	send_pixel(x0 + r, y0, color);
	send_pixel(x0 - r, y0, color);

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x;

		send_pixel(x0 + x, y0 + y, color);
		send_pixel(x0 - x, y0 + y, color);
		send_pixel(x0 + x, y0 - y, color);
		send_pixel(x0 - x, y0 - y, color);
		send_pixel(x0 + y, y0 + x, color);
		send_pixel(x0 - y, y0 + x, color);
		send_pixel(x0 + y, y0 - x, color);
		send_pixel(x0 - y, y0 - x, color);
	}

	finish();
}

template <typename T>
void swap(T& a1, T& a2)
{
	T temp = a1;
	a1 = a2;
	a2 = temp;
}

void Display::draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const Color color)
{
	const bool steep = abs(y1 - y0) > abs(x1 - x0);

	if (steep)
	{
		swap(x0, y0);
		swap(x1, y1);
	}

	if (x0 > x1)
	{
		swap(x0, x1);
		swap(y0, y1);
	}

	int16_t dx = x1 - x0;
	int16_t dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1)
		ystep = 1;
	else
		ystep = -1;

	ss::low();

	for (; x0 <= x1; x0++)
	{
		if (steep)
			send_pixel(y0, x0, color);
		else
			send_pixel(x0, y0, color);

		err -= dy;
		if (err < 0)
		{
			y0 += ystep;
			err += dx;
		}
	}

	ss::high();
}

void Display::send_hline(uint8_t x, uint8_t y, uint8_t len, Color color)
{
	send_addr_window(x, y, len, 1);
	send_pixels(color, len);
}

void Display::send_vline(uint8_t x, uint8_t y, uint8_t len, Color color)
{
	send_addr_window(x, y, 1, len);
	send_pixels(color, len);
}

void Display::draw_hline(uint8_t x, uint8_t y, uint8_t len, Color color)
{
	ss::low();
	send_hline(x,  y,  len, color);
	ss::high();
}

void Display::draw_vline(uint8_t x, uint8_t y, uint8_t len, Color color)
{
	ss::low();
	send_vline(x, y, len, color);
	ss::high();
}

void Display::fill_circle(uint8_t x0, uint8_t y0, uint8_t r, Color color)
{
	ss::low();

	send_vline(x0, y0 - r, 2 * r + 1, color);

	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;
	int16_t px = x;
	int16_t py = y;

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x;

		// These checks avoid double-drawing certain lines, important
		// for the SSD1306 library which has an INVERT drawing mode.
		if (x < y + 1)
		{
			send_vline(x0 + x, y0 - y, 2 * y + 1, color);
			send_vline(x0 - x, y0 - y, 2 * y + 1, color);
		}

		if (y != py)
		{
			send_vline(x0 + py, y0 - px, 2 * px + 1, color);
			send_vline(x0 - py, y0 - px, 2 * px + 1, color);
			py = y;
		}
		px = x;
	}

	ss::high();
}

void Display::draw_raster(const uint8_t* raster, uint8_t x, uint8_t y, uint8_t w, uint8_t h, Color color, Color bgcolor)
{
	ss::low();

	send_addr_window(x, y, w, h);

	Color curr_color = bgcolor;
	while (true)
	{
		const uint8_t num_pixels = pgm_read_byte(raster++);

		if (num_pixels == 0)
			break;

		send_pixels(curr_color, num_pixels);

		if (curr_color == color)
			curr_color = bgcolor;
		else
			curr_color = color;
	}

	ss::high();
}
