#pragma once

#include "usart.h"
#include "iopin.h"
#include "ring.h"

enum PedalEvent : uint8_t
{
	evNone,

	evFtswInit,
	evFtswBtn1Down,
	evFtswBtn1Up,
	evFtswBtn2Down,
	evFtswBtn2Up,
	evFtswBtn3Down,
	evFtswBtn3Up,
	evFtswBtn4Down,
	evFtswBtn4Up,
	evFtswDoubleBtn,
	evFtswOff,

	evExpInit,
	evExpBtnDown,
	evExpBtnUp,
	evExpPosition,
	evExpOff,
};

enum PedalLED : uint8_t
{
	ledFtswQA3		= 0,
	ledFtswMode3	= 1,
	ledFtswQA2		= 2,
	ledFtswMiddle	= 3,
	ledFtswQA1		= 4,
	ledFtswMode1	= 5,
	ledFtswModeTuner = 6,
	ledFtswMode2	= 7,

	ledExpGreen		= 11,
	ledExpRed		= 13,
};

class Pedals: public Usart<3>
{
public:

	bool		ftsw_present = false;
	bool		exp_present = false;

	// last known states of buttons and rockers
	bool		ftsw_btn1 = false;
	bool		ftsw_btn2 = false;
	bool		ftsw_btn3 = false;
	bool		ftsw_btn4 = false;
	uint16_t	exp_position = 0;
	bool		exp_btn = false;

	uint8_t		exp_leds = 0;
	uint16_t	ftsw_number = 0;
	uint8_t		ftsw_leds = 0;

	Pedals()
	{
		reset();
	}

	PedalEvent get_event();

	void set_ftsw_number(const uint16_t num)
	{
		new_ftsw_number = num;

		// we can only show numbers from 0 to 999
		// on a 3 digit LED display
		while (new_ftsw_number > 999)
			new_ftsw_number -= 1000;
	}

	void clear_ftsw_number()
	{
		new_ftsw_number = FTSW_NUM_CLEAR;
	}

	void set_led(const PedalLED led);
	void clear_led(const PedalLED led);

	void reset();
	void clear();
	void clear_ftsw();
	void clear_exp();

private:

	enum {
		FTSW_NUM_UNKNOWN	= 0xfffe,
		FTSW_NUM_CLEAR		= 0xffff,
	};

	uint8_t		received = 0;
	uint8_t		receive[7];
	uint8_t		expected = 0;

	uint16_t	min_pos = 0xffff;
	uint16_t	max_pos = 0;

	uint16_t	new_ftsw_number = FTSW_NUM_CLEAR;
	uint8_t		new_ftsw_leds	= 0;
	uint8_t		new_exp_leds	= 0;

	uint16_t	last_reception	= 0;

	uint8_t		ftsw_error_cnt	= 0;
	uint8_t		exp_error_cnt	= 0;

	ring<PedalEvent, 10>	events;

	uint8_t		send_buff[8];

	bool consume(const uint8_t byte);
	void update_button_state(const PedalEvent event);
	bool send_message();
	void parse_message();

	bool send(const uint8_t b);

	void refresh_ftsw_display();
	void refresh_ftsw_leds();
	void refresh_exp_leds();
};
