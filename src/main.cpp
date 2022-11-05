#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "iopin.h"
#include "timera.h"
#include "usart.h"
#include "avrdbg.h"

#include "pedals.h"

using led = IoPin<'C', 6>;
using btn = IoPin<'C', 7>;
using tmr = TimerA<1>;

// init CPU clock and PORTMUX
void hw_init()
{
	// get us up to 24MHz; warp speed!
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.OSCHFCTRLA = CLKCTRL_AUTOTUNE_bm | CLKCTRL_FRQSEL_24M_gc;

	// TimerA 1 PWM to PORTC
	PORTMUX.TCAROUTEA = PORTMUX_TCA1_0_bm;

	// USART 3 is on ALT1: TX->PB4 RX->PB5
	PORTMUX.USARTROUTEA = PORTMUX_USART3_0_bm;
}

int main()
{
	hw_init();

	// setup the debug log
	dbgInit();
	dprint("\nI live...\n");
	
	// config the onboard LED and button
	led::dir_out();
	led::invert();
	
	btn::dir_in();
	btn::pullup();

	// enable PWM on CMP2 of TimerA1
	tmr::set_clock_div(tmr::div256);
	//tmr::set_period(0x400);
	//tmr::set_pwm_duty<2>(0x010);
	//tmr::enable_pwm<2>();
	tmr::start();
	
	Pedals pedals;
	
	while (true)
	{
		const PedalEvent event = pedals.get_event();
		if (event != evNone)
		{
			if (event == evFtswBtn1Down)
			{
				pedals.set_led(ledFtswModeTuner);
				pedals.set_led(ledExpGreen);
			}
			else if (event == evFtswBtn1Up)
			{
				pedals.clear_led(ledFtswModeTuner);
				pedals.clear_led(ledExpGreen);
			}
			else if (event == evFtswBtn2Down)
			{
				pedals.set_led(ledFtswQA1);
			}
			else if (event == evFtswBtn2Up)
			{
				pedals.clear_led(ledFtswQA1);
			}
			else if (event == evFtswBtn3Down)
			{
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
				pedals.set_led(ledFtswQA3);
			}
			else if (event == evFtswBtn4Up)
			{
				pedals.clear_led(ledFtswQA3);
			}
			else if (event == evExpPosition)
			{
				uint16_t num = pedals.exp_position;
				pedals.set_ftsw_number(num);
				
				if (num & 0x1000)
					pedals.set_led(ledFtswMode1);
				else
					pedals.clear_led(ledFtswMode1);

				if (num & 0x800)
					pedals.set_led(ledFtswMode2);
				else
					pedals.clear_led(ledFtswMode2);
				
				if (num & 0x400)
					pedals.set_led(ledFtswMode3);
				else
					pedals.clear_led(ledFtswMode3);
				
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
