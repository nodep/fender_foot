#pragma once

#define _AVR_IO_H_
#define __extension__

#pragma warning (push, 0)
#include <avr/ioavr128da48.h>
#pragma warning (pop)

#pragma warning (disable : 4061)
#pragma warning (disable : 4514)
#pragma warning (disable : 5214)

void loop_until_bit_is_set(const uint16_t byte, const uint8_t bit);

#undef CPU_CCP
#undef CLKCTRL
#undef VPORTA
#undef PORTA
#undef USART0
#undef PORTMUX
#undef TCA0

extern uint8_t		CPU_CCP;
extern CLKCTRL_t	CLKCTRL;
extern VPORT_t		VPORTs[8];
extern PORT_t		PORTs[8];
extern USART_t		USARTs[5];
extern PORTMUX_t	PORTMUX;
extern TCA_t		TCAs[2];

#define PORTA		PORTs[0]
#define VPORTA		VPORTs[0]
#define USART0		USARTs[0]
#define TCA0		TCAs[0]
