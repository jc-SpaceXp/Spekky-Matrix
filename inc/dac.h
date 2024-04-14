#ifndef DAC_H
#define DAC_H

#include <stdint.h>

enum DacDataAlignment { EightBit, TwelveBitRight, TwelveBitLeft };

struct DacTxRegisters {
	volatile uint32_t* dac_8bit;
	volatile uint32_t* dac_12bit_right;
	volatile uint32_t* dac_12bit_left;
};

void trigger_dac(struct DacTxRegisters dac, uint16_t data, enum DacDataAlignment dac_align);
void trigger_dac_byte_transfer(volatile uint32_t* dac_tx, uint8_t data);

#endif /* DAC_H */
