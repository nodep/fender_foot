#pragma once

#include <avr/io.h>

enum TimerA_Prescale
{
	div1 = TCA_SINGLE_CLKSEL_DIV1_gc,
	div2 = TCA_SINGLE_CLKSEL_DIV2_gc,
	div4 = TCA_SINGLE_CLKSEL_DIV4_gc,
	div8 = TCA_SINGLE_CLKSEL_DIV8_gc,
	div16 = TCA_SINGLE_CLKSEL_DIV16_gc,
	div64 = TCA_SINGLE_CLKSEL_DIV64_gc,
	div256 = TCA_SINGLE_CLKSEL_DIV256_gc,
	div1024 = TCA_SINGLE_CLKSEL_DIV1024_gc,
};

template <uint8_t TimerNum, TimerA_Prescale div>
class TimerA
{
private:
	static TCA_SINGLE_t& get_tca()
	{
		return (&TCA0)[TimerNum].SINGLE;
	}

public:

	static void start()
	{
		get_tca().CTRLA |= TCA_SINGLE_ENABLE_bm;
	}

	static void stop()
	{
		get_tca().CTRLA &= ~TCA_SINGLE_ENABLE_bm;
	}

	static void set_prescale()
	{
		get_tca().CTRLA = static_cast<uint8_t>((get_tca().CTRLA & 0x81) | div);
	}

	// beware overflows!
	static uint16_t ticks2ms(const uint16_t ticks)
	{
		if (div == div1)
			return ticks / (F_CPU/1000);
		else if (div == div2)
			return ticks * 2 / (F_CPU/1000);
		else if (div == div4)
			return ticks * 4 / (F_CPU/1000);
		else if (div == div8)
			return ticks * 8 / (F_CPU/1000);
		else if (div == div16)
			return ticks * 16 / (F_CPU/1000);
		else if (div == div64)
			return ticks * 64 / (F_CPU/1000);
		else if (div == div256)
			return ticks * 256 / (F_CPU/1000);
		else if (div == div1024)
			return ticks * 1024 / (F_CPU/1000);

		return 0;
	}

	static uint16_t ms2ticks(const uint16_t ms)
	{
		if (div == div1)
			return ms * (F_CPU/1000);
		else if (div == div2)
			return ms * (F_CPU/1000) / 2;
		else if (div == div4)
			return ms * (F_CPU/1000) / 4;
		else if (div == div8)
			return ms * (F_CPU/1000) / 8;
		else if (div == div16)
			return ms * (F_CPU/1000) / 16;
		else if (div == div64)
			return ms * (F_CPU/1000) / 64;
		else if (div == div256)
			return ms * (F_CPU/1000) / 256;
		else if (div == div1024)
			return ms * (F_CPU/1000) / 1024;

		return 0;
	}

	static register16_t& cnt()
	{
		return get_tca().CNT;
	}

	static void set_period(const uint16_t period)
	{
		get_tca().PER = period;
	}

	template <uint8_t Channel>
	static void enable_pwm()
	{
		static_assert(Channel < 3);

		const uint16_t prevCtrlb = (get_tca().CTRLB & 0x78);

		get_tca().CTRLB = (TCA_SINGLE_CMP0EN_bm << Channel)
						| TCA_SINGLE_WGMODE_SINGLESLOPE_gc
						| prevCtrlb;
	}

	template <uint8_t Channel>
	static void set_pwm_duty(const uint16_t cmp)
	{
		static_assert(Channel < 3);

		(&(get_tca().CMP0))[Channel] = cmp;
	}
};
