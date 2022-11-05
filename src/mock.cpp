#include <avr/io.h>
#include <util/delay.h>

uint8_t CPU_CCP;
CLKCTRL_t CLKCTRL;
PORTMUX_t PORTMUX;

VPORT_t VPORTs[8];
PORT_t PORTs[8];
USART_t USARTs[8];
TCA_t TCAs[2];

void _delay_ms(const uint16_t d)
{
	d;
}

void loop_until_bit_is_set(const uint16_t byte, const uint8_t bit)
{
	byte;
	bit;
}
