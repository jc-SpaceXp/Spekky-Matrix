#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "greatest.h"
#include "dac_suite.h"

#include "dac.h"

static uint32_t some_dac_tx_reg_8bit = 0xFFFF;

TEST data_alignment_tx_8_bit(void)
{
	uint8_t input_data = 0x01;
	trigger_dac_byte_transfer(&some_dac_tx_reg_8bit, input_data);
	ASSERT_EQ(some_dac_tx_reg_8bit, (uint16_t) input_data);
	PASS();
}


SUITE(dac_driver)
{
	RUN_TEST(data_alignment_tx_8_bit);
}

