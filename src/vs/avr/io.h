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
bool bit_is_clear(const uint16_t byte, const uint8_t bit);

#undef CPU_CCP
#undef CLKCTRL
#undef VPORTA
#undef PORTA
#undef USART0
#undef USART1
#undef USART2
#undef USART3
#undef USART4
#undef PORTMUX
#undef TCA0
#undef SPI0
#undef SPI1

extern uint8_t		CPU_CCP;
extern CLKCTRL_t	CLKCTRL;
extern VPORT_t		VPORTs[8];
extern PORT_t		PORTs[8];
extern USART_t		USARTs[5];
extern PORTMUX_t	PORTMUX;
extern TCA_t		TCAs[2];
extern SPI_t		SPIs[2];

#define PORTA		PORTs[0]
#define VPORTA		VPORTs[0]
#define USART0		USARTs[0]
#define TCA0		TCAs[0]
#define USART0		USARTs[0]
#define USART1		USARTs[1]
#define USART2		USARTs[2]
#define USART3		USARTs[3]
#define USART4		USARTs[4]
#define SPI0		SPIs[0]
#define SPI1		SPIs[1]
