#include <stdint.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "avrdbg.h"
#include "pedals.h"

const uint8_t ST_INIT	= 0xFD;
const uint8_t ST_ACK	= 0xFD;
const uint8_t ST_NACK	= 0xFE;
const uint8_t ST_BUTTON	= 0xFC;
const uint8_t ST_LIGHTS	= 0xF8;
const uint8_t ST_EXP	= 0xFB;
const uint8_t ST_END	= 0xFF;

const uint8_t ID_EXP	= 0x0C;
const uint8_t ID_FTSW	= 0x08;

const uint8_t LED_FTSW	= 0x70;	// the LEDS on the expression pedal
const uint8_t LED_DISP2	= 0x72;	// left digit on the LED display
const uint8_t LED_DISP1	= 0x74;	// middle digit on the LED display
const uint8_t LED_DISP0	= 0x76;	// right digit on the LED display
const uint8_t LED_EXP	= 0x70;	// the LEDS on the foot switch

enum {
 	MAX_STREAM_BYTES = 5,
	MAX_ERROR_CNT = 5,
};

struct
{
	const PedalEvent event;
	const uint8_t stream[MAX_STREAM_BYTES];

	const PedalEvent get_event() const
	{
		return static_cast<PedalEvent>(pgm_read_byte(&event));
	}
	
	const uint8_t get_stream(const uint8_t istream) const
	{
		return pgm_read_byte(&stream[istream]);
	}
	
} const messages[] PROGMEM =
{
	{evExpPosition,		{ST_EXP,	ID_EXP,		ST_END}},

	{evFtswBtn4Down,	{ST_BUTTON, ID_FTSW,	0x7D, 0x7C, 0x09}},
	{evFtswBtn3Down,	{ST_BUTTON, ID_FTSW,	0x7D, 0x7D, 0x08}},
	{evFtswBtn2Down,	{ST_BUTTON, ID_FTSW,	0x7D, 0x7E, 0x0B}},
	{evFtswBtn1Down,	{ST_BUTTON, ID_FTSW,	0x7D, 0x7F, 0x0A}},
	{evFtswBtn4Up,		{ST_BUTTON, ID_FTSW,	0x7F, 0x7C, 0x0B}},
	{evFtswBtn3Up,		{ST_BUTTON, ID_FTSW,	0x7F, 0x7D, 0x0A}},
	{evFtswBtn2Up,		{ST_BUTTON, ID_FTSW,	0x7F, 0x7E, 0x09}},
	{evFtswBtn1Up,		{ST_BUTTON, ID_FTSW,	0x7F, 0x7F, 0x08}},
	{evExpBtnDown,		{ST_BUTTON, ID_EXP,		0x7C, 0x00, 0x70}},
	{evExpBtnUp,		{ST_BUTTON, ID_EXP,		0x7E, 0x00, 0x72}},

	{evFtswInit,		{ST_INIT,	ID_FTSW,	0x03, 0x0B, ST_END}},
	{evExpInit,			{ST_INIT,	ID_EXP,		0x01, 0x0D, ST_END}},

	{evNone,			{ST_END}}
};

static const uint8_t digit_segments[10] PROGMEM =
{
	0x5F,			// 0
	0x14,			// 1
	0x4E | 0x80,	// 2
	0x56 | 0x80,	// 3
	0x15 | 0x80,	// 4
	0x53 | 0x80,	// 5
	0x5B | 0x80,	// 6
	0x16,			// 7
	0x5f | 0x80,	// 8
	0x17 | 0x80,	// 9
};

bool Pedals::consume(const uint8_t byte)
{
	if (is_status(byte))
	{
		receive[0] = byte;
		received = 1;

		switch (byte)
		{
		case ST_INIT:		expected = 4;				break;
		case ST_BUTTON:		expected = 5;				break;
		case ST_EXP:		expected = 6;				break;
		default:
			dprint("unexpected cmd %02x\n", byte);
			expected = received = 0;
			break;
		}
	}
	else if (expected > received)
	{
		receive[received++] = byte;
	}
	else
	{
		dprint("unexpected %02x\n", byte);
		received = expected = 0;
	}

	return expected == received  &&  received > 0;
}

PedalEvent Pedals::get_event()
{
	poll();

	// if we have no incoming message
	if (received == 0)
	{
		refresh_ftsw_num();
		refresh_ftsw_leds();
		refresh_exp_leds();
	}

	if (events.empty())
		return evNone;

	return events.pop();
}

void Pedals::reset()
{
	// TODO: this function should not block
	IoPin<'B', 4>::dir_out();	// TX is out
	IoPin<'B', 4>::clear();		// lo
	_delay_ms(1000);

	IoPin<'B', 4>::dir_out();	// TX is out
	IoPin<'B', 5>::dir_in();	// RX is in
}

void Pedals::clear()
{
	received = expected = 0;

	min = 0xffff;
	max = 0;

	new_ftsw_number = FTSW_NUM_CLEAR;
	ftsw_number = FTSW_NUM_UNKNOWN;
}

void Pedals::set_led(const PedalLED led)
{
	// expression or footswitch leds?
	if (led > 7)
		new_exp_leds |= (1 << (led - 8));
	else
		new_ftsw_leds |= (1 << led);
}

void Pedals::clear_led(const PedalLED led)
{
	// expression or footswitch leds?
	if (led > 7)
		new_exp_leds &= ~(1 << (led - 8));
	else
		new_ftsw_leds &= ~(1 << led);
}

void Pedals::update_state(const PedalEvent fired_event)
{
	// get the rocket state
	switch (fired_event)
	{
	case evExpPosition:
		// get the 14 bit position of the rocker
		exp_position = receive[3];
		exp_position <<= 7;
		exp_position |= receive[4];

		// update the range of the rocker (poor man's calibration)
		if (exp_position < min) min = exp_position;
		if (exp_position > max) max = exp_position;
		
		// subtract the minimum from the position
		exp_position -= min;
		break;
	case evFtswInit:		ftsw_present = true;		break;
	case evExpInit:			exp_present = true;			break;
	case evExpBtnDown:		exp_btn = true;				break;
	case evExpBtnUp:		exp_btn = false;			break;
	case evFtswBtn1Down:	ftsw_btn1 = true;			break;
	case evFtswBtn1Up:		ftsw_btn1 = false;			break;
	case evFtswBtn2Down:	ftsw_btn2 = true;			break;
	case evFtswBtn2Up:		ftsw_btn2 = false;			break;
	case evFtswBtn3Down:	ftsw_btn3 = true;			break;
	case evFtswBtn3Up:		ftsw_btn3 = false;			break;
	case evFtswBtn4Down:	ftsw_btn4 = true;			break;
	case evFtswBtn4Up:		ftsw_btn4 = false;			break;
	default:
		break;
	}
}

PedalEvent Pedals::parse_message()
{
	// find the event in the table
	PedalEvent fired_event = evNone;
	uint8_t istream = 0;
	uint8_t imsg = 0;

	while (messages[imsg].get_event() != fired_event)
	{
		if (messages[imsg].get_stream(istream) == receive[istream])
		{
			++istream;
		}
		else if (messages[imsg].get_stream(istream) < receive[istream])
		{
			++imsg;
			istream = 0;
		}
		else
		{
			break;
		}

		// completed a message?
		if (istream == MAX_STREAM_BYTES
			|| (messages[imsg].get_stream(istream) == ST_END  &&  istream))
		{
			// checksum
			uint8_t cs = 0;
			for (uint8_t c = 1; c < expected; c++)
				cs ^= receive[c];

			// confirm to the sender
			send(cs == 0 ? ST_ACK : ST_NACK);

			expected = received = 0;

			if (cs == 0)
			{
				fired_event = messages[imsg].get_event();

				update_state(fired_event);
			}

			break;
		}
	}

	return fired_event;
}

void Pedals::poll()
{
	uint8_t byte = 0;
	while (read_byte(byte)  &&  consume(byte))
	{
		const PedalEvent ev = parse_message();
		if (ev != evNone)
		{
			if (events.is_full())
			{
				// TODO: error state
			}
				
			events.push(ev);
		}
	}
}

bool Pedals::send(const uint8_t b)
{
	send_byte(b);

	uint8_t d = 0;
	while (!read_byte(d))
		;

	if (b != d)
		dprint("send failed:%02X d:%02X\n", b, d);
	
	return b == d;
}

bool Pedals::send_message()
{
	// send the message
	uint8_t checksum = send_buff[0];
	for (const uint8_t b : send_buff)
	{
		if (!send(b))
			return false;
		
		checksum ^= b;
	}

	send(checksum);

	// wait for ACK
	uint8_t ack = 0;
	while (!read_byte(ack))
		;

	if (ack == ST_ACK)
		return true;

	dprint("bad checksum 0x%02x\n", ack);

	if (send_buff[1] == ID_FTSW)
	{
		if (++ftsw_error_cnt == MAX_ERROR_CNT)
		{
			ftsw_present = false;
			if (!events.is_full())
				events.push(evFtswOff);
		}
	}
		
	if (send_buff[1] == ID_EXP)
	{
		if (++exp_error_cnt == MAX_ERROR_CNT)
		{
			exp_present = false;
			if (!events.is_full())
				events.push(evExpOff);
		}
	}
	
	return false;
}

void Pedals::refresh_ftsw_leds()
{
	if (new_ftsw_leds != ftsw_leds  &&  ftsw_present)
	{
		// this message sets the individual LEDs on the foot switch
		send_buff[0] = ST_LIGHTS;
		send_buff[1] = ID_FTSW;
		send_buff[2] = LED_FTSW;
		send_buff[3] = new_ftsw_leds & 0x7F;
		send_buff[4] = 0;
		send_buff[5] = 0;
		send_buff[6] = 0;
		send_buff[7] = 0;
		
		if (new_ftsw_leds & 0x80)
			send_buff[2] = LED_FTSW + 1;

		if (send_message())
			ftsw_leds = new_ftsw_leds;
	}
}

void Pedals::refresh_exp_leds()
{
	if (new_exp_leds != exp_leds  &&  exp_present)
	{
		// this message sets the individual LEDs on the foot switch
		send_buff[0] = ST_LIGHTS;
		send_buff[1] = ID_EXP;
		send_buff[2] = LED_EXP;
		send_buff[3] = new_exp_leds;
		send_buff[4] = 0;
		send_buff[5] = 0;
		send_buff[6] = 0;
		send_buff[7] = 0;
		
		if (send_message())
			exp_leds = new_exp_leds;
	}
}

void Pedals::refresh_ftsw_num()
{
	if (new_ftsw_number != ftsw_number  &&  ftsw_present)
	{
		// this message clears the LED display
		send_buff[0] = ST_LIGHTS;
		send_buff[1] = ID_FTSW;
		send_buff[2] = LED_DISP2;
		send_buff[3] = 0;
		send_buff[4] = LED_DISP1;
		send_buff[5] = 0;
		send_buff[6] = LED_DISP0;
		send_buff[7] = 0;

		// we modify the message in case we don't want
		// to clear the display, but show a number instead
		if (new_ftsw_number != FTSW_NUM_CLEAR)
		{
			uint8_t d0 = new_ftsw_number % 10;
			uint8_t d2 = static_cast<uint8_t>(new_ftsw_number / 10);
			uint8_t d1 = d2 % 10;
			d2 /= 10;

			uint8_t segments;
			
			if (d2)
			{
				segments = pgm_read_byte(&digit_segments[d2]);
				if (segments & 0x80)
					send_buff[2] = LED_DISP2 + 1;
				send_buff[3] = segments & 0x7f;
			}

			if (d1  ||  d2)
			{
				segments = pgm_read_byte(&digit_segments[d1]);
				if (segments & 0x80)
					send_buff[4] = LED_DISP1 + 1;
				send_buff[5] = segments & 0x7f;
			}

			segments = pgm_read_byte(&digit_segments[d0]);
			if (segments & 0x80)
				send_buff[6] = LED_DISP0 + 1;
			send_buff[7] = segments & 0x7f;
		}

		if (send_message())
			ftsw_number = new_ftsw_number;
	}
}
