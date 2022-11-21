#include <stdint.h>

#include "display.h"

#define ST77XX_NOP 0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID 0x04
#define ST77XX_RDDST 0x09

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_PTLON 0x12
#define ST77XX_NORON 0x13

#define ST77XX_INVOFF 0x20
#define ST77XX_INVON 0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29
#define ST77XX_CASET 0x2A
#define ST77XX_RASET 0x2B
#define ST77XX_RAMWR 0x2C
#define ST77XX_RAMRD 0x2E

#define ST77XX_PTLAR 0x30
#define ST77XX_TEOFF 0x34
#define ST77XX_TEON 0x35
#define ST77XX_MADCTL 0x36
#define ST77XX_COLMOD 0x3A

#define ST77XX_MADCTL_MY 0x80
#define ST77XX_MADCTL_MX 0x40
#define ST77XX_MADCTL_MV 0x20
#define ST77XX_MADCTL_ML 0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1 0xDA
#define ST77XX_RDID2 0xDB
#define ST77XX_RDID3 0xDC
#define ST77XX_RDID4 0xDD

// Some ready-made 16-bit ('565') color settings:
#define ST77XX_BLACK 0x0000
#define ST77XX_WHITE 0xFFFF
#define ST77XX_RED 0xF800
#define ST77XX_GREEN 0x07E0
#define ST77XX_BLUE 0x001F
#define ST77XX_CYAN 0x07FF
#define ST77XX_MAGENTA 0xF81F
#define ST77XX_YELLOW 0xFFE0
#define ST77XX_ORANGE 0xFC00

// Some register settings
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH 0x04

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define ST_CMD_DELAY 0x80 // special signifier for command lists

static const uint8_t PROGMEM initCommands[] =
{          							// 7735R init
    ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
      150,                          //     150 ms delay
    ST77XX_SLPOUT,    ST_CMD_DELAY, //  2: Out of sleep mode, 0 args, w/delay
      255,                          //     500 ms delay
    ST7735_FRMCTR1, 3,              //  3: Framerate ctrl - normal mode, 3 arg:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3,              //  4: Framerate ctrl - idle mode, 3 args:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6,              //  5: Framerate - partial mode, 6 args:
      0x01, 0x2C, 0x2D,             //     Dot inversion mode
      0x01, 0x2C, 0x2D,             //     Line inversion mode
    ST7735_INVCTR,  1,              //  6: Display inversion ctrl, 1 arg:
      0x07,                         //     No inversion
    ST7735_PWCTR1,  3,              //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                         //     -4.6V
      0x84,                         //     AUTO mode
    ST7735_PWCTR2,  1,              //  8: Power control, 1 arg, no delay:
      0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
    ST7735_PWCTR3,  2,              //  9: Power control, 2 args, no delay:
      0x0A,                         //     Opamp current small
      0x00,                         //     Boost frequency
    ST7735_PWCTR4,  2,              // 10: Power control, 2 args, no delay:
      0x8A,                         //     BCLK/2,
      0x2A,                         //     opamp current small & medium low
    ST7735_PWCTR5,  2,              // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1,  1,              // 12: Power control, 1 arg, no delay:
      0x0E,
    ST77XX_INVOFF,  0,              // 13: Don't invert display, no args
    ST77XX_MADCTL,  1,              // 14: Mem access ctl (directions), 1 arg:
      0xC8,                         //     row/col addr, bottom-top refresh
    ST77XX_COLMOD,  1,              // 15: set color mode, 1 arg, no delay:
      0x05,                       	//     16-bit color

    ST77XX_CASET,   4,              // 16: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x7F,                   //     XEND = 127
    ST77XX_RASET,   4,              // 17: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x9F,                 	//     XEND = 159

    ST7735_GMCTRP1, 16,       		// 18: Gamma Adjustments (pos. polarity), 16 args + delay:
      0x02, 0x1c, 0x07, 0x12,       //     (Not entirely necessary, but provides
      0x37, 0x32, 0x29, 0x2d,       //      accurate colors)
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16,       		// 19: Gamma Adjustments (neg. polarity), 16 args + delay:
      0x03, 0x1d, 0x07, 0x06,       //     (Not entirely necessary, but provides
      0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST77XX_NORON,     ST_CMD_DELAY, // 20: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST77XX_DISPON,    ST_CMD_DELAY, // 21: Main screen turn on, no args w/delay
      100,							//     100 ms delay
	  
	ST77XX_MADCTL,   1,				// 22: change MADCTL color filter
	  0xC0,

	0x00,							// EOF
};                        

void Display::Display()
{
    const uint8_t* addr = initCommands;
	uint8_t cmd;

	while ((cmd = pgm_read_byte(addr++)) != 0)
	{
		uint8_t numArgs = pgm_read_byte(addr++);	// Number of args to follow
		uint16_t ms = numArgs & ST_CMD_DELAY;		// If hibit set, delay follows args
		numArgs &= ~ST_CMD_DELAY;					// Mask out delay bit
		sendInitCommand(cmd, addr, numArgs);
		addr += numArgs;

		if (ms)
		{
			ms = pgm_read_byte(addr++);		// Read post-command delay time (ms)
			if (ms == 255)
				ms = 500;
			while (ms--)
				_delay_ms(1);
		}
	}
}

inline void spiWrite16(const uint16_t w)
{
	spi::send(static_cast<uint8_t>(w >> 8));
	spi::send(static_cast<uint8_t>(w));
}

void spiWrite32(const uint32_t l)
{
    spi::send(static_cast<uint8_t>(l >> 24));
    spi::send(static_cast<uint8_t>(l >> 16));
    spi::send(static_cast<uint8_t>(l >> 8));
    spi::send(static_cast<uint8_t>(l));
}

inline void writeCommand(const uint8_t cmd)
{
	dc::low();
	spi::send(cmd);
	dc::high();
}

void sendInitCommand(const uint8_t commandByte, const uint8_t* dataBytes, const uint8_t numDataBytes)
{
	ss::low();

	writeCommand(commandByte);

	for (int i = 0; i < numDataBytes; i++)
		spi::send(pgm_read_byte(dataBytes++));

	ss::high();
}

void setAddrWindow(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	const uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
	const uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);

	writeCommand(ST77XX_CASET); // Column addr set
	spiWrite32(xa);

	writeCommand(ST77XX_RASET); // Row addr set
	spiWrite32(ya);

	writeCommand(ST77XX_RAMWR); // write to RAM
}

void writeColor(const uint16_t color, uint16_t len)
{
	for (uint16_t c = 0; c < len; c++)
		spiWrite16(color);
}

void fillScreen(const uint16_t color)
{
	ss::low();
	setAddrWindow(0, 0, 128, 160);
	writeColor(color, 128*160);
	ss::high();
}
