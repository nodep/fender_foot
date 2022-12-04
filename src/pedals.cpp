#include <stdint.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "avrdbg.h"
#include "pedals.h"
#include "watch.h"

const uint8_t CMD_LED	= 0xF8;
const uint8_t CMD_DBTN	= 0xFA;
const uint8_t CMD_POS	= 0xFB;
const uint8_t CMD_BTN	= 0xFC;
const uint8_t CMD_INIT	= 0xFD;

const uint8_t ID_EXP	= 0x0C;
const uint8_t ID_FTSW	= 0x08;

const uint8_t ACK		= 0xFD;
const uint8_t ERROR		= 0xFE;

const uint8_t LED_FTSW	= 0x70;	// the LEDS on the expression pedal
const uint8_t LED_DISP2	= 0x72;	// left digit on the LED display
const uint8_t LED_DISP1	= 0x74;	// middle digit on the LED display
const uint8_t LED_DISP0	= 0x76;	// right digit on the LED display
const uint8_t LED_EXP	= 0x70;	// the LEDS on the foot switch

const bool SHOW_LEADING_ZEROS = false;

enum
{
	// number of consecutive NACK-ed messages before we give up on a pedal
	MAX_ERROR_CNT = 5,

	// how many milliseconds do we wait from the last reception
	// until we start refreshing LEDs
	REFRESH_DELAY = 0,
};

// these are LED values representing numbers
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
	// is this a command byte?
	if (byte & 0x80)
	{
		receive[0] = byte;
		received = 1;

		switch (byte)
		{
		case CMD_INIT:	expected = 4;	break;
		case CMD_BTN:	expected = 5;	break;
		case CMD_POS:	expected = 6;	break;
		case CMD_DBTN:	expected = 7;	break;
		default:
			dprint("nxpd cmd %02X\n", byte);
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
		dprint("nxpd %02X\n", byte);
		received = expected = 0;
	}

	return expected == received  &&  received > 0;
}

PedalEvent Pedals::get_event()
{
	uint8_t byte = 0;
	while (read_byte(byte)  &&  consume(byte))
		parse_message();

	if (received == 0			// no active reception
		&&  events.empty()		// no unhandled events
		&&  (REFRESH_DELAY == 0  ||  Watch::has_ms_passed_since(REFRESH_DELAY, last_reception)))
	{
		refresh_ftsw_display();
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

	enable(false, false);	// disable RX and TX

	IoPin<'B', 4>::dir_out();	// TX is out
	IoPin<'B', 4>::low();		// lo

	_delay_ms(1000);

	IoPin<'B', 4>::dir_out();	// TX is out
	IoPin<'B', 5>::dir_in();	// RX is in

	set_baud(31250);
	enable(true, true);		// enable RX and TX

	clear();
}

void Pedals::clear()
{
	events.clear();

	received = expected = 0;

	clear_ftsw();
	clear_exp();
}

void Pedals::clear_ftsw()
{
	ftsw_error_cnt = 0;
	ftsw_present = false;

	// these force a refresh of LEDs
	ftsw_number = FTSW_NUM_UNKNOWN;
	ftsw_leds = static_cast<uint8_t>(new_ftsw_leds + 1);

	ftsw_btn1 = ftsw_btn2 = ftsw_btn3 = ftsw_btn4 = false;
}

void Pedals::clear_exp()
{
	exp_error_cnt = 0;
	exp_present = false;

	// these force a refresh of LEDs
	exp_leds = static_cast<uint8_t>(new_exp_leds + 1);

	exp_btn = false;
	exp_position = 0;
}

void Pedals::set_led(const PedalLED led)
{
	// expression or foot switch leds?
	if (led > 7)
		new_exp_leds |= (1 << (led - 8));
	else
		new_ftsw_leds |= (1 << led);
}

void Pedals::clear_led(const PedalLED led)
{
	// expression or foot switch leds?
	if (led > 7)
		new_exp_leds &= ~(1 << (led - 8));
	else
		new_ftsw_leds &= ~(1 << led);
}

PedalEvent ftsw_btn_change(const uint8_t* pchange)
{
	const uint16_t change_code = *reinterpret_cast<const uint16_t*>(pchange);

	switch (change_code)
	{
	case 0x7C7D:	return evFtswBtn4Down;
	case 0x7D7D:	return evFtswBtn3Down;
	case 0x7E7D:	return evFtswBtn2Down;
	case 0x7F7D:	return evFtswBtn1Down;
	case 0x7C7F:	return evFtswBtn4Up;
	case 0x7D7F:	return evFtswBtn3Up;
	case 0x7E7F:	return evFtswBtn2Up;
	case 0x7F7F:	return evFtswBtn1Up;
	case 0x7C:		return evExpBtnDown;
	case 0x7E:		return evExpBtnUp;
	default:
		break;
	}

	return evNone;
}

void Pedals::update_button_state(const PedalEvent event)
{
	switch (event)
	{
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

void Pedals::parse_message()
{
	// checksum
	uint8_t cs = 0;
	for (uint8_t c = 1; c < received; c++)
		cs ^= receive[c];

	// confirm to the sender
	send(cs == 0 ? ACK : ERROR);

	expected = received = 0;

	if constexpr(REFRESH_DELAY != 0)
		last_reception = Watch::cnt();

	PedalEvent event = evNone;

	if (receive[0] == CMD_INIT)
	{
		if (receive[1] == ID_FTSW)
		{
			event = evFtswInit;
			clear_ftsw();
			ftsw_present = true;
		}
		else if (receive[1] == ID_EXP)
		{
			event = evExpInit;
			clear_exp();
			exp_present = true;
		}
	}
	else if (receive[0] == CMD_BTN)
	{
		event = ftsw_btn_change(receive + 2);
		update_button_state(event);
	}
	else if (receive[0] == CMD_DBTN)
	{
		event = evFtswDoubleBtn;
	}
	else if (receive[0] == CMD_POS)
	{
		event = evExpPosition;

		// get the 14 bit position of the rocker
		exp_position = receive[3];
		exp_position <<= 7;
		exp_position |= receive[4];

		// update the range of the rocker (poor man's calibration)
		if (exp_position < min_pos) min_pos = exp_position;
		if (exp_position > max_pos) max_pos = exp_position;

		// subtract the minimum from the position
		exp_position -= min_pos;
	}

	if (event != evNone)
	{
		events.safe_push(event);

		if (event == evFtswDoubleBtn)
		{
			const PedalEvent first = ftsw_btn_change(receive + 2);
			const PedalEvent second = ftsw_btn_change(receive + 4);

			update_button_state(first);
			update_button_state(second);

			events.safe_push(first);
			events.safe_push(second);
		}
	}
}

bool Pedals::send(const uint8_t b)
{
	// send the byte
	send_byte(b);

	// wait for the byte to appear on RX because
	// these are connected on the same bus
	const uint16_t started = Watch::cnt();
	while (!Watch::has_ms_passed_since(1, started))
	{
		uint8_t d = 0;
		if (read_byte(d))
		{
			if (b != d)
				dprint("send failed:%02X d:%02X\n", b, d);

			return b == d;
		}
	}

	dprint("send timeout\n");

	return true;
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
	const uint16_t started = Watch::cnt();
	uint8_t ack = 0;
	bool byte_read = false;
	while (!Watch::has_ms_passed_since(2, started))
	{
		if (read_byte(ack))
		{
			if (ack == ACK)
			{
				if (send_buff[1] == ID_FTSW)
					ftsw_error_cnt = 0;
				else if (send_buff[1] == ID_EXP)
					exp_error_cnt = 0;

				return true;
			}

			byte_read = true;

			dprint("bad checksum %02x\n", ack);

			break;
		}
	}

	if (!byte_read)
		dprint("ack timeout\n");

	// check if we have too many errors and
	// need to give up on a pedal
	if (send_buff[1] == ID_FTSW)
	{
		if (++ftsw_error_cnt == MAX_ERROR_CNT)
		{
			clear_ftsw();
			events.safe_push(evFtswOff);
		}
	}
	else if (send_buff[1] == ID_EXP)
	{
		if (++exp_error_cnt == MAX_ERROR_CNT)
		{
			clear_exp();
			events.safe_push(evExpOff);
		}
	}

	return false;
}

void Pedals::refresh_ftsw_leds()
{
	if (new_ftsw_leds != ftsw_leds  &&  ftsw_present)
	{
		// this message sets the individual LEDs on the foot switch
		send_buff[0] = CMD_LED;
		send_buff[1] = ID_FTSW;
		send_buff[2] = LED_FTSW;
		send_buff[3] = static_cast<uint8_t>(new_ftsw_leds & 0x7F);
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
		send_buff[0] = CMD_LED;
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

void Pedals::refresh_ftsw_display()
{
	if (new_ftsw_number != ftsw_number  &&  ftsw_present)
	{
		// this message clears the LED display
		send_buff[0] = CMD_LED;
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
			uint8_t d0 = static_cast<uint8_t>(new_ftsw_number % 10);
			uint8_t d2 = static_cast<uint8_t>(new_ftsw_number / 10);
			uint8_t d1 = static_cast<uint8_t>(d2 % 10);
			d2 /= 10;

			uint8_t segments;

			if (SHOW_LEADING_ZEROS  ||  d2)
			{
				segments = pgm_read_byte(&digit_segments[d2]);
				if (segments & 0x80)
					send_buff[2] = LED_DISP2 + 1;
				send_buff[3] = static_cast<uint8_t>(segments & 0x7f);
			}

			if (SHOW_LEADING_ZEROS  ||  d1  ||  d2)
			{
				segments = pgm_read_byte(&digit_segments[d1]);
				if (segments & 0x80)
					send_buff[4] = LED_DISP1 + 1;
				send_buff[5] = static_cast<uint8_t>(segments & 0x7f);
			}

			segments = pgm_read_byte(&digit_segments[d0]);
			if (segments & 0x80)
				send_buff[6] = LED_DISP0 + 1;
			send_buff[7] = static_cast<uint8_t>(segments & 0x7f);
		}

		if (send_message())
			ftsw_number = new_ftsw_number;
	}
}
