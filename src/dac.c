#include "dac.h"

static uint16_t align_dac_input_data(enum DacDataAlignment dac_align, uint16_t data)
{
	uint16_t aligned_data = (uint8_t) data;

	if (dac_align == TwelveBitRight) {
		aligned_data = data & 0xFFF;
	} else if (dac_align == TwelveBitLeft) {
		aligned_data = data << 4;
	}

	return aligned_data;
}

void trigger_dac(struct DacTxRegisters dac, uint16_t data, enum DacDataAlignment dac_align)
{
	volatile uint32_t* dac_tx = dac.dac_8bit;

	if (dac_align == TwelveBitRight) {
		dac_tx = dac.dac_12bit_right;
	} else if (dac_align == TwelveBitLeft) {
		dac_tx = dac.dac_12bit_left;
	}

	*dac_tx = align_dac_input_data(dac_align, data);
}

void trigger_dac_byte_transfer(volatile uint32_t* dac_tx, uint8_t data)
{
	*dac_tx = data;
}
