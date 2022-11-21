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
#include "spimaster.h"

using led = IoPin<'C', 6>;
using btn = IoPin<'C', 7>;

// SPI pins
using mosi	= IoPin<'A', 4>;
using sck	= IoPin<'A', 6>;
using ss	= IoPin<'A', 7>;
using rst	= IoPin<'B', 0>;
using dc	= IoPin<'B', 1>;

using spi	= SpiMaster<0, 7>;

// init the CPU clock and PORTMUX
void hw_init()
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

#define TWI0_BAUD(F_SCL, T_RISE) ((((((float)F_CPU / (float)F_SCL)) - 10 - ((float)F_CPU * T_RISE / 1000000))) / 2)

void twi_test()
{
	// debugging pin
	using pb4 = IoPin<'B', 4>;
	pb4::dir_out();
	pb4::high();
	pb4::low();
	pb4::high();
	pb4::low();

	// set up the I2C
	using sda = IoPin<'A', 2>;
	using scl = IoPin<'A', 3>;

	sda::low();
	sda::dir_out();
	scl::low();
	scl::dir_out();

	TWI0.CTRLA = TWI_SDAHOLD_500NS_gc;
	TWI0.MBAUD = TWI0_BAUD(100000, 0);
	TWI0.MCTRLA  |= TWI_ENABLE_bm;
	TWI0.MSTATUS |= TWI_BUSSTATE_IDLE_gc;

	_delay_ms(100);

	const uint8_t addr = 0x78;
	while (true)
	{
		pb4::high();

		// start by sending the address
		TWI0.MADDR = addr | 1;

		loop_until_bit_is_set(TWI0.MSTATUS, TWI_WIF_bp);

		if (bit_is_clear(TWI0.MSTATUS, TWI_RXACK_bp))
			dprint("found 0x%02x\n", addr);
		else
			dprint("no 0x%02x\n", addr);

		TWI0.MCTRLB = TWI_MCMD_STOP_gc;

		pb4::low();

		_delay_ms(100);
	}
}

int main()
{
	hw_init();

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

	// init
	displayInit(initCommands);

	// show something
	const uint16_t cols[] = {
		ST77XX_BLACK,
		ST77XX_WHITE,
		ST77XX_RED,
		ST77XX_GREEN,
		ST77XX_BLUE,
		ST77XX_CYAN,
		ST77XX_MAGENTA,
		ST77XX_YELLOW,
		ST77XX_ORANGE,
	};

	uint8_t ndx = 0;
	while (true)
	{
		const uint16_t n = Watch::cnt();
		fillScreen(cols[ndx % 9]);
		const uint16_t dur = Watch::cnt() - n;
		dprint("dur: %d\n", (uint16_t)Watch::ticks2ms(dur));

		ndx++;

		_delay_ms(1000);
	}
}
