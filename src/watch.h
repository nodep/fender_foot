#pragma once

#include "timera.h"

class Watch : public TimerA<1, TimerA_Prescale::div1024>
{
public:
	static uint32_t ticks2ms(const uint32_t ticks)
	{
		return ticks * get_div() / (F_CPU / 1000);
	}

	static uint32_t ms2ticks(const uint32_t ms)
	{
		return ms * (F_CPU / 1000) / get_div();
	}

	static bool has_ms_passed_since(const uint16_t ms, const uint16_t since)
	{
		return static_cast<uint32_t>(cnt() - since) >= ms2ticks(ms);
	}

	static uint16_t ms_since(const uint16_t since)
	{
		return ticks2ms(cnt() - since);
	}
};
