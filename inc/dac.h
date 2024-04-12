#ifndef DAC_H
#define DAC_H

#include <stdint.h>

void trigger_dac_byte_transfer(uint32_t* dac_tx, uint8_t data);

#endif /* DAC_H */
