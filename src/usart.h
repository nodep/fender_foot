#pragma once

template <uint8_t UsartNum>
class Usart
{
protected:
	static_assert(UsartNum <= 5);

	constexpr static USART_t& get_usart()
	{
		if constexpr (UsartNum == 0)
			return USART0;
		else if constexpr (UsartNum == 1)
			return USART1;
		else if constexpr (UsartNum == 2)
			return USART2;
		else if constexpr (UsartNum == 3)
			return USART3;

		return USART4;
	}

public:
	static void set_baud(const uint32_t baud)
	{
		const uint16_t b = static_cast<uint16_t>((float)(F_CPU * 64 / (16 * (float)baud)) + 0.5);

		get_usart().BAUD = b;
	}

	static void enable(const bool tx, const bool rx)
	{
		get_usart().CTRLB = static_cast<uint8_t>( (rx ? USART_RXEN_bm : 0)
												| (tx ? USART_TXEN_bm : 0) );
	}

	static void send_byte(const uint8_t b)
	{
		loop_until_bit_is_set(get_usart().STATUS, USART_DREIF_bp);
		get_usart().TXDATAL = b;
	}

	static bool read_byte(uint8_t& b)
	{
		if (get_usart().STATUS & USART_RXCIF_bm)
		{
			b = get_usart().RXDATAL;
			return true;
		}
		
		return false;
	}
};
