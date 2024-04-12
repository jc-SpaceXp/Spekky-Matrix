#include "dac.h"

void trigger_dac_byte_transfer(uint32_t* dac_tx, uint8_t data)
{
	*dac_tx = data;
}
