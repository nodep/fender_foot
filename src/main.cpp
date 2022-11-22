#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <util/delay.h>
#include <avr/pgmspace.h>

#include "iopin.h"
#include "timera.h"
#include "usart.h"
#include "avrdbg.h"
#include "watch.h"

#include "pedals.h"
#include "display.h"

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

int main()
{
	init_hw();

	Display::init();

	Display::fill_screen(colBlack);

	while (true)
	{
		const uint16_t start = Watch::cnt();

		//Display::draw_line(0, 0, 128, 160, colGreen);
		//Display::draw_line(0, 160, 128, 0, colYellow);
		//Display::fill_rect(0, 150, 128, 10, colGreen);
		Display::fill_circle(64, 64, 63, colBlue);

		//for (uint8_t r = 1; r <= 63; r++)
		//	Display::draw_circle(63, 63, r, colBlue);

		const uint16_t dur = Watch::cnt() - start;
		dprint("dur: %d\n", (uint16_t)Watch::ticks2ms(dur));

		_delay_ms(1000);
	}
}
