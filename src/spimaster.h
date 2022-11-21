#pragma once

#include <avr/io.h>

template <uint8_t SpiNum, uint8_t Speed>
class SpiMaster
{
protected:
	constexpr static SPI_t& get_spi()
	{
		if (SpiNum == 0)
			return SPI0;

		return SPI1;
	}

	constexpr static uint8_t speed_flags()
	{
		if (Speed == 0)
			return SPI_PRESC_DIV128_gc;
		else if (Speed == 1)
			return SPI_PRESC_DIV64_gc;
		else if (Speed == 2)
			return SPI_PRESC_DIV64_gc | SPI_CLK2X_bm;
		else if (Speed == 3)
			return SPI_PRESC_DIV16_gc;
		else if (Speed == 4)
			return SPI_PRESC_DIV16_gc | SPI_CLK2X_bm;
		else if (Speed == 5)
			return SPI_PRESC_DIV4_gc;
		
		return SPI_PRESC_DIV4_gc | SPI_CLK2X_bm;
	}

public:
	static void init()
	{
		get_spi().CTRLB = SPI_SSD_bm;
		get_spi().CTRLA = SPI_MASTER_bm | speed_flags() | SPI_ENABLE_bm;
	}

    // TODO: RX not implemented yet
	static void send(const uint8_t byte)
	{
		get_spi().DATA = byte;
		loop_until_bit_is_set(get_spi().INTFLAGS, SPI_IF_bp);
	}
};
