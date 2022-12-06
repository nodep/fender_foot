#pragma once

#include <avr/io.h>

enum TimerA_Prescale : uint8_t
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

template <uint8_t TimerNum, TimerA_Prescale prescale>
class TimerA
{
private:
	static_assert(TimerNum <= 1, "We don't have that timer");

	constexpr static TCA_SINGLE_t& get_tca()
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
		get_tca().CTRLA = static_cast<uint8_t>((get_tca().CTRLA & 0x81) | prescale);
	}

	static register16_t& cnt()
	{
		return get_tca().CNT;
	}

	static void set_period(const uint16_t period)
	{
		get_tca().PER = period;
	}

	constexpr static uint16_t get_div()
	{
		if constexpr(prescale == div1)
			return 1;
		else if constexpr(prescale == div2)
			return 2;
		else if constexpr(prescale == div4)
			return 4;
		else if constexpr(prescale == div8)
			return 8;
		else if constexpr(prescale == div16)
			return 16;
		else if constexpr(prescale == div64)
			return 64;
		else if constexpr(prescale == div256)
			return 256;

		// default is div1024
		return 1024;
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
