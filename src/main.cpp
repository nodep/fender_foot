#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <util/delay.h>
#include <avr/pgmspace.h>

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

const uint8_t PROGMEM pot_ring[] =
{
 23, 11, 42, 19, 36, 23, 32, 27, 28, 31, 25, 33, 23, 35, 20, 39,
 17, 16,  9, 16, 15, 14, 15, 14, 14, 12, 19, 12, 13, 11, 23, 11,
 11, 11, 25, 11,  9, 11, 27, 11,  8, 10, 29, 10,  7, 10, 31, 10,
  6,  9, 33,  9,  5,  9, 35,  9,  4,  9, 35,  9,  3,  9, 37,  9,
  2,  9, 37,  9,  2,  8, 39,  8,  2,  8, 39,  8,  1,  9, 39, 17,
 41, 16, 41, 16, 41, 16, 41, 16, 41, 16, 41, 16, 41, 16, 41, 16,
 41, 17, 39,  9,  1,  8, 39,  8,  2,  8, 39,  8,  2,  9, 37,  9,
  2,  9, 37,  9,  3,  9, 35,  9,  4,  9, 35,  9,  5,  9, 33,  9,
  6, 10, 31, 10,  7, 10, 29, 10,  8, 11, 27, 11,  9, 11, 25, 11,
 11, 11, 23, 11, 13, 12, 19, 12, 14, 14, 15, 14, 15, 16,  9, 16,
 17, 39, 20, 35, 23, 33, 25, 31, 28, 27, 32, 23, 36, 19, 42, 11,
 23, 0
};

#define PI 3.14159265

int main()
{
	init_hw();

	Display::init();

	Display d;

	//uint16_t start;

	while (true)
	{
		WindowRGB<64, 64> canvas(colBlack);
		fill(canvas, colBlack);
		for (Coord r = 19; r < 22; ++r)
			draw_circle(canvas, 31, 31, r, colWhite);
		print(canvas, "ujaaa!", true, 20, 20, colGreen, colBlack);
		d.blit(canvas, 0, 0);

		_delay_ms(500);

		/*
		dprint("-----------------\n");

		start = Watch::cnt();
		fill_circle(d, 20, 20, 19, colBlue);
		draw_line(d, 0, 0, 39, 39, colBlue);
		dprint("old: %d\n", Watch::ms_since(start));

		_delay_ms(1000);

		start = Watch::cnt();
		Window<40, 40> w(colBlack);
		fill_circle(w, 20, 20, 19, colGreen);
		draw_line(w, 0, 0, 39, 39, colGreen);
		dprint("draw16: %d\n", Watch::ms_since(start));

		start = Watch::cnt();
		Display::blit(w, 0, 0);
		dprint("blt16: %d\n", Watch::ms_since(start));

		_delay_ms(1000);

		start = Watch::cnt();
		WindowRGB<40, 40> w16(colBlack);
		fill_circle(w16, 20, 20, 19, colRed);
		draw_line(w16, 0, 0, 39, 39, colRed);
		dprint("drawRGB: %d\n", Watch::ms_since(start));

		start = Watch::cnt();
		Display::blit(w16, 0, 0);
		dprint("bltRGB: %d\n", Watch::ms_since(start));

		_delay_ms(1000);

		//char buff[256];
		//size_t i;
		//for (i = 0; i < sizeof(buff) - 16; ++i)
		//	buff[i] = i + 16;
		//buff[i] = '\0';
		//Display::print(buff, false, 0, 0, colWhite, colBlack);

		//Display::draw_raster(pot_ring, 5, 20, 57, 57, colRed, colBlack);
		*/
	}
}
