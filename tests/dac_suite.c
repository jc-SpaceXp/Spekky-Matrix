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

struct DacTxTest {
	uint16_t input_data;
	enum DacDataAlignment dac_align;
	uint16_t expected_result;
};

static uint32_t check_dac_tx_reg(struct DacTxRegisters dac, enum DacDataAlignment dac_align)
{
	uint32_t* dac_tx = dac.dac_8bit;
	if (dac_align == TwelveBitRight) {
		dac_tx = dac.dac_12bit_right;
	} else if (dac_align == TwelveBitLeft) {
		dac_tx = dac.dac_12bit_left;
	}

	return *dac_tx;
}

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


TEST dac_data_tx_alignment(struct DacTxTest dac_inputs)
{
	trigger_dac(some_dac_regs, dac_inputs.input_data, dac_inputs.dac_align);
	ASSERT_EQ_FMT(check_dac_tx_reg(some_dac_regs, dac_inputs.dac_align)
	             , dac_inputs.expected_result
	             , "%X");
	PASS();
}


TEST snprintf_return_val(bool sn_error)
{
	ASSERT_FALSE(sn_error);
	PASS();
}

void loop_test_dac_data_tx_alignment(void)
{
	struct DacTxTest dac_tx[3] = {
		{ 0x2E, EightBit, 0x2E }
		, { 0xF31A, TwelveBitRight, 0x031A}
		, { 0x0442, TwelveBitLeft,  0x4420}
	};
	for (int i = 0; i < 3; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(dac_data_tx_alignment, dac_tx[i]);
	}
}

SUITE(dac_driver)
{
	RUN_TEST(data_alignment_tx_8bit);
	RUN_TEST(data_alignment_tx_12bit_right);
	RUN_TEST(data_alignment_tx_12bit_left);
	// looped test
	loop_test_dac_data_tx_alignment();
}

