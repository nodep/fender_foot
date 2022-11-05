#include <avr/io.h>

#if _DEBUG

#include "avrdbg.h"
#include "iopin.h"
#include "usart.h"

using DebugUsart = Usart<1>;

#ifndef _WIN32
static int serial_putchar(char c, FILE*)
{
	DebugUsart::send_byte(c);
	return 0;
}
#endif

void dbgInit()
{
#ifndef _WIN32
	// we can't use FDEV_SETUP_STREAM, so we improvise
	static FILE mydbgout;

	mydbgout.buf = nullptr;
	mydbgout.put = serial_putchar;
	mydbgout.get = nullptr;
	mydbgout.flags = _FDEV_SETUP_WRITE;
	mydbgout.udata = 0;

    stdout = &mydbgout;
#endif

	// USART1.TX is on C0 on avr128da48,
	// and we have to configure it for output
	IoPin<'C', 0>::dir_out();
	
	// baud rate calc
	DebugUsart::set_baud(230400);
	DebugUsart::enable(true, false);
}

void printi(uint32_t i)
{
	char buff[11];
	char* start = buff + 9;
	buff[9] = '0';
	buff[10] = '\0';
	for (uint8_t cnt = 9; cnt <= 9  &&  i; --cnt)
	{
		buff[cnt] = '0' + i % 10;
		if (buff[cnt] != '0')
			start = buff + cnt;
		i /= 10;
	}
	puts(start);
}

#endif	// _DEBUG
