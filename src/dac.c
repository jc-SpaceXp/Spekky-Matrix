#include "dac.h"

static uint16_t align_dac_input_data(enum DacDataAlignment dac_align, uint16_t data)
{
	uint16_t aligned_data = data;

	if (dac_align == TwelveBitRight) {
		aligned_data = data & 0xFFF;
	}

	return aligned_data;
}

void trigger_dac_transfer(uint32_t* dac_tx, uint16_t data, enum DacDataAlignment dac_align)
{
	*dac_tx = align_dac_input_data(dac_align, data);
}

void trigger_dac_byte_transfer(uint32_t* dac_tx, uint8_t data)
{
	*dac_tx = data;
}
