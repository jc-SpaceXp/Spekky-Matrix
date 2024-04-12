#ifndef DAC_H
#define DAC_H

#include <stdint.h>

enum DacDataAlignment { EightBit, TwelveBitRight, TwelveBitLeft };

void trigger_dac_transfer(uint32_t* dac_tx, uint16_t data, enum DacDataAlignment dac_align);
void trigger_dac_byte_transfer(uint32_t* dac_tx, uint8_t data);

#endif /* DAC_H */
