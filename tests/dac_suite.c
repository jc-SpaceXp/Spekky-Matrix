#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "greatest.h"
#include "dac_suite.h"

#include "dac.h"

static uint32_t some_dac_tx_reg_8bit = 0xFFFF;
static uint32_t some_dac_tx_reg_12bit = 0xFFFF;

TEST data_alignment_tx_8_bit(void)
{
	uint8_t input_data = 0x01;
	trigger_dac_byte_transfer(&some_dac_tx_reg_8bit, input_data);
	ASSERT_EQ(some_dac_tx_reg_8bit, (uint16_t) input_data);
	PASS();
}

TEST data_alignment_tx_12_bit_right(void)
{
	uint16_t input_data = 0x9192;
	trigger_dac_transfer(&some_dac_tx_reg_12bit, input_data, TwelveBitRight);
	ASSERT_EQ(some_dac_tx_reg_12bit, input_data & 0xFFF); // only 12 bits are kept
	PASS();
}

SUITE(dac_driver)
{
	RUN_TEST(data_alignment_tx_8_bit);
	RUN_TEST(data_alignment_tx_12_bit_right);
}

