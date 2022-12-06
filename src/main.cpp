#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "iopin.h"
#include "timera.h"
#include "usart.h"
#include "avrdbg.h"
#include "watch.h"

#include "pedals.h"
#include "display.h"
#include "graphtext.h"

using led = IoPin<'C', 6>;
using btn = IoPin<'C', 7>;

// init the CPU clock, PORTMUX, and onboard LED and button
void init_hw()
{
	CPU_CCP = CCP_IOREG_gc;
#if   F_CPU == 1000000
	CLKCTRL.OSCHFCTRLA = CLKCTRL_AUTOTUNE_bm | CLKCTRL_FRQSEL_1M_gc;
#elif F_CPU == 2000000
	CLKCTRL.OSCHFCTRLA = CLKCTRL_AUTOTUNE_bm | CLKCTRL_FRQSEL_2M_gc;
#elif F_CPU == 3000000
	CLKCTRL.OSCHFCTRLA = CLKCTRL_AUTOTUNE_bm | CLKCTRL_FRQSEL_3M_gc;
#elif F_CPU == 4000000
	CLKCTRL.OSCHFCTRLA = CLKCTRL_AUTOTUNE_bm | CLKCTRL_FRQSEL_4M_gc;
#elif F_CPU == 8000000
	CLKCTRL.OSCHFCTRLA = CLKCTRL_AUTOTUNE_bm | CLKCTRL_FRQSEL_8M_gc;
#elif F_CPU == 12000000
	CLKCTRL.OSCHFCTRLA = CLKCTRL_AUTOTUNE_bm | CLKCTRL_FRQSEL_12M_gc;
#elif F_CPU == 16000000
	CLKCTRL.OSCHFCTRLA = CLKCTRL_AUTOTUNE_bm | CLKCTRL_FRQSEL_16M_gc;
#elif F_CPU == 20000000
	CLKCTRL.OSCHFCTRLA = CLKCTRL_AUTOTUNE_bm | CLKCTRL_FRQSEL_20M_gc;
#elif F_CPU == 24000000
	CLKCTRL.OSCHFCTRLA = CLKCTRL_AUTOTUNE_bm | CLKCTRL_FRQSEL_24M_gc;
#else
	#error Unknown F_CPU setting
#endif

	// TimerA 1 PWM to PORTC
	PORTMUX.TCAROUTEA = PORTMUX_TCA1_0_bm;

	// USART 3 is on ALT1: TX->PB4 RX->PB5
	PORTMUX.USARTROUTEA = PORTMUX_USART3_0_bm;

	// setup the debug log
	dbgInit();
	dprint("\nI live...\n");

	// config the on-board LED and button
	led::dir_out();
	led::invert();

	btn::dir_in();
	btn::pullup();

	// setup our main clock
	Watch::set_prescale();
	Watch::start();
}

void test_pedals()
{
	Pedals pedals;

	uint8_t mode = 0;
	uint16_t num = 0;
	while (true)
	{
		const PedalEvent event = pedals.get_event();
		if (event != evNone)
		{
			if (pedals.ftsw_btn3  &&  pedals.ftsw_btn4)
				pedals.set_led(ledFtswMiddle);
			else
				pedals.clear_led(ledFtswMiddle);

			if (event == evFtswBtn1Down)
			{
				pedals.set_led(ledFtswModeTuner);
				pedals.set_led(ledExpGreen);

				if (++mode == 4)
					mode = 1;

				pedals.clear_led(ledFtswMode1);
				pedals.clear_led(ledFtswMode2);
				pedals.clear_led(ledFtswMode3);

				if (mode == 1)
					pedals.set_led(ledFtswMode1);
				else if (mode == 2)
					pedals.set_led(ledFtswMode2);
				else if (mode == 3)
					pedals.set_led(ledFtswMode3);

				num = 0;
				pedals.set_ftsw_number(num);
			}
			else if (event == evFtswBtn1Up)
			{
				pedals.clear_led(ledFtswModeTuner);
				pedals.clear_led(ledExpGreen);
			}
			else if (event == evFtswBtn2Down)
			{
				num += 100;
				pedals.set_ftsw_number(num);
				pedals.set_led(ledFtswQA1);
			}
			else if (event == evFtswBtn2Up)
			{
				pedals.clear_led(ledFtswQA1);
			}
			else if (event == evFtswBtn3Down)
			{
				num += 10;
				pedals.set_ftsw_number(num);
				pedals.set_led(ledFtswQA2);
				pedals.set_led(ledExpRed);
			}
			else if (event == evFtswBtn3Up)
			{
				pedals.clear_led(ledFtswQA2);
				pedals.clear_led(ledExpRed);
			}
			else if (event == evFtswBtn4Down)
			{
				num += 1;
				pedals.set_ftsw_number(num);
				pedals.set_led(ledFtswQA3);
			}
			else if (event == evFtswBtn4Up)
			{
				pedals.clear_led(ledFtswQA3);
			}
			else if (event == evExpPosition)
			{
				if (!pedals.ftsw_present)
					dprint("pos %d\n", pedals.exp_position);

				num = static_cast<uint16_t>(pedals.exp_position >> 3);
				if (num > 999)
					num = 999;
				pedals.set_ftsw_number(num);
			}
			else if (event == evExpBtnDown)
			{
				pedals.set_led(ledFtswMiddle);
				pedals.set_led(ledExpRed);
				pedals.set_led(ledExpGreen);
			}
			else if (event == evExpBtnUp)
			{
				pedals.clear_led(ledFtswMiddle);
				pedals.clear_led(ledExpRed);
				pedals.clear_led(ledExpGreen);
			}
			else if (event == evExpInit)
				dprint("exp online\n");
			else if (event == evFtswInit)
				dprint("ftsw online\n");
			else if (event == evExpOff)
				dprint("exp offline\n");
			else if (event == evFtswOff)
				dprint("ftsw offline\n");
		}
	}
}

const int DIAL_ARC_POINTS = 85;

struct { Coord x, y; } const dial_arc[DIAL_ARC_POINTS] PROGMEM = {
{ 6,34},{ 5,33},{ 4,32},{ 4,31},{ 3,30},{ 3,29},{ 2,28},{ 2,27},{ 2,26},{ 1,25},
{ 1,24},{ 1,23},{ 1,22},{ 1,21},{ 1,20},{ 1,19},{ 1,18},{ 1,17},{ 2,16},{ 2,15},
{ 2,14},{ 3,13},{ 3,12},{ 4,11},{ 4,10},{ 5, 9},{ 6, 8},{ 7, 7},{ 7, 7},{ 8, 6},
{ 9, 5},{10, 4},{11, 4},{12, 3},{13, 3},{14, 2},{15, 2},{16, 2},{17, 1},{18, 1},
{19, 1},{20, 1},{21, 1},{22, 1},{23, 1},{24, 1},{25, 1},{26, 2},{27, 2},{28, 2},
{29, 3},{30, 3},{31, 4},{32, 4},{33, 5},{34, 6},{35, 7},{35, 7},{36, 8},{37, 9},
{38,10},{38,11},{39,12},{39,13},{40,14},{40,15},{40,16},{41,17},{41,18},{41,19},
{41,20},{41,21},{41,22},{41,23},{41,24},{41,25},{40,26},{40,27},{40,28},{39,29},
{39,30},{38,31},{38,32},{37,33},{36,34}
};

template <typename Canvas>
struct ThickBrush
{
	using Transaction = typename Canvas::Transaction;

	Canvas& canvas;

	ThickBrush(Canvas& w)
		: canvas(w)
	{}

	void pixel(Coord x, Coord y, Color col)
	{
		canvas.pixel(x, y, col);
		canvas.pixel(x+1, y, col);
		canvas.pixel(x-1, y, col);
		canvas.pixel(x, y+1, col);
		canvas.pixel(x, y-1, col);
	}
};

template <typename Canvas, typename ColorT>
void draw_dial(Canvas& canvas, Coord x, Coord y, uint8_t position, ColorT col)
{
	static_assert(Canvas::Width >= 43  &&  Canvas::Height >= 36, "Canvas is too small for draw_dial()");

	ThickBrush<Canvas> tb(canvas);

	{
		[[maybe_unused]] typename Canvas::Transaction t;

		// draw the arc
		for (const auto& pt : dial_arc)
			tb.pixel(pgm_read_byte(&pt.x) + x, pgm_read_byte(&pt.y) + y, col);
	}

	// draw the dial line
	const Coord x1 = pgm_read_byte(&dial_arc[position].x) + x;
	const Coord y1 = pgm_read_byte(&dial_arc[position].y) + y;

	draw_line(tb, x + 20, y + 20, x1, y1, col);
}

void draw_dial_at(const char* name, uint8_t position, Coord x, Coord y, Color col)
{
	{
		Window<43, 36> win(colBlack);
		draw_dial(win, 0, 0, position, col);
		Display::blit(win, x + 11, y);
	}

	Window<64, 8> win(colBlack);
	const uint16_t width = get_text_width_small(name);
	const Coord textx = width > 64 ? 0 : (64 - width) / 2;
	print_small(win, name, textx, 0, col, colBlack);
	Display::blit(win, x, y + 38);
}

void refresh_screen()
{
	Display d;

	// the battery
	const uint8_t battery = rand() % 0x80;
	fill_rect(d, 0, 0, battery, 3, colGreen);
	fill_rect(d, battery, 0, 127 - battery, 3, colBlack);

	// the selected effect name
	{
		const char* names[] = {"Reverb", "Chorus", "Delay"};
		const uint8_t namendx = rand() % 3;
		const uint16_t width = get_text_width_large(names[namendx]);

		const Coord x = width > 128 ? 0 : (128 - width) / 2;

		Window<128, 19> win(colBlack);
		print_large(win, names[namendx], x, 0, colWhite);
		d.blit(win, 0, 10);
	}

	// draw the knobs
	draw_dial_at("green",  rand() % DIAL_ARC_POINTS, 0, 36, colGreen);
	draw_dial_at("blue", rand() % DIAL_ARC_POINTS, 64, 36, colBlue);
	draw_dial_at("red",  rand() % DIAL_ARC_POINTS, 0, 95, colRed);
	draw_dial_at("white", rand() % DIAL_ARC_POINTS, 64, 95, colWhite);
}

int main() 
{
	init_hw();

	Display d;

	d.init();

	fill(d, colBlack);

	while (true)
	{
		refresh_screen();

		_delay_ms(500);
	}
}
