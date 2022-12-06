#pragma once

#include <avr/io.h>

template <char Port, uint8_t PinNum>
class IoPin
{
protected:

	static_assert(Port >= 'A'  &&  Port <= 'F', "Bad Port in IoPin template argument");
	static_assert(PinNum <= 7, "Bad pin number n IoPin template argument");

	//////////////////
	// helpers
	//////////////////
	constexpr static register8_t& get_pinctrl()
	{
		return (&((&PORTA)[Port - 'A'].PIN0CTRL))[PinNum];
	}

	constexpr static VPORT_t& get_vport()
	{
		return (&VPORTA)[Port - 'A'];
	}

	static constexpr uint8_t bitmask = 1 << PinNum;

public:

	//////////////////
	// configuration
	//////////////////
	static void dir_out()
	{
		get_vport().DIR |= bitmask;
	}

	static void dir_in()
	{
		get_vport().DIR &= ~bitmask;
	}

	static void invert()
	{
		get_pinctrl() |= PORT_INVEN_bm;
	}

	static void non_invert()
	{
		get_pinctrl() &= ~PORT_INVEN_bm;
	}

	//////////////////
	// output
	//////////////////
	static void high()
	{
		get_vport().OUT |= bitmask;
	}

	static void low()
	{
		get_vport().OUT &= ~bitmask;
	}

	static void toggle()
	{
		get_vport().IN |= bitmask;
	}

	static void set_value(const bool value)
	{
		if (value)
			high();
		else
			low();
	}

	//////////////////
	// input
	//////////////////
	static bool in()
	{
		return get_vport().IN & bitmask;
	}

	static void pullup()
	{
		get_pinctrl() |= PORT_PULLUPEN_bm;
	}

	static void pullup_off()
	{
		get_pinctrl() &= ~PORT_PULLUPEN_bm;
	}
};
