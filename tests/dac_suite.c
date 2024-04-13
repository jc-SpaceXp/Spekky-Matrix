#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "greatest.h"
#include "dac_suite.h"

#include "dac.h"

static uint32_t some_dac_tx_reg_8bit = 0xFFFF;
static uint32_t some_dac_tx_reg_12bit = 0xFFFF;
static uint32_t some_dac_tx_reg_12bit_right = 0xFFFF;
static uint32_t some_dac_tx_reg_12bit_left = 0xFFFF;

static struct DacTxRegisters some_dac_regs = {
	&some_dac_tx_reg_8bit, &some_dac_tx_reg_12bit_right, &some_dac_tx_reg_12bit_left
};

TEST data_alignment_tx_8bit(void)
{
	uint8_t input_data = 0x01;
	trigger_dac_byte_transfer(&some_dac_tx_reg_8bit, input_data);
	ASSERT_EQ(some_dac_tx_reg_8bit, (uint16_t) input_data);
	PASS();
}

TEST data_alignment_tx_12bit_right(void)
{
	uint16_t input_data = 0x9192;
	trigger_dac_transfer(&some_dac_tx_reg_12bit, input_data, TwelveBitRight);
	ASSERT_EQ(some_dac_tx_reg_12bit, input_data & 0xFFF); // only 12 bits are kept
	PASS();
}

TEST data_alignment_tx_12bit_left(void)
{
	uint16_t input_data = 0x0192;
	trigger_dac_transfer(&some_dac_tx_reg_12bit, input_data, TwelveBitLeft);
	ASSERT_EQ(some_dac_tx_reg_12bit, 0x1920); // only upper 12 bits of reg are filled
	PASS();
}

TEST data_alignment_tx_8bit_alt(void)
{
	uint8_t input_data = 0x51;
	trigger_dac(some_dac_regs, input_data, EightBit);
	ASSERT_EQ(some_dac_tx_reg_8bit, (uint16_t) input_data);
	PASS();
}

TEST data_alignment_tx_12bit_right_alt(void)
{
	uint16_t input_data = 0x3191;
	trigger_dac(some_dac_regs, input_data, TwelveBitRight);
	ASSERT_EQ(some_dac_tx_reg_12bit_right, input_data & 0xFFF); // only 12 bits are kept
	PASS();
}

TEST data_alignment_tx_12bit_left_alt(void)
{
	uint16_t input_data = 0x0412;
	trigger_dac(some_dac_regs, input_data, TwelveBitLeft);
	ASSERT_EQ(some_dac_tx_reg_12bit_left, 0x4120); // only upper 12 bits of reg are filled
	PASS();
}

SUITE(dac_driver)
{
	RUN_TEST(data_alignment_tx_8bit);
	RUN_TEST(data_alignment_tx_12bit_right);
	RUN_TEST(data_alignment_tx_12bit_left);
	RUN_TEST(data_alignment_tx_8bit_alt);
	RUN_TEST(data_alignment_tx_12bit_right_alt);
	RUN_TEST(data_alignment_tx_12bit_left_alt);
}

