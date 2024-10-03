#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "greatest.h"
#include "mic_data_suite.h"

#include "hw_verification/i2s_dma_buffer_gdb_output_sine_1khz.h"

#include "mic_data_processing.h"

struct DmaConversionLoop {
	enum MicChannel channel_select;
	float expected;
	unsigned int input_offset;
	unsigned int input_bytes;
};

TEST snprintf_return_val(bool sn_error)
{
	ASSERT_FALSE(sn_error);
	PASS();
}


TEST i2s_limits_conversion(void)
{
	float test_buffer[4] = { 0 };
	int16_t limits_buffer[8] = { 0xFFFF, 0xFF00, 0x00, 0x00
	                           , 0x00, 0x00, 0x00, 0x00 };
	// data is transmitted MSB first
	// therefore bits 31-8 are filled rather than 23-0
	uint32_t expected[2] = {4294967040LU, 0};
	dma_i2s_halfword_to_word_complex_conversion(limits_buffer, test_buffer, 8, L);

	for (int i = 0; i < 2; ++i) {
		uint16_t first_bytes = limits_buffer[i * 4];
		uint16_t second_bytes = limits_buffer[(i * 4) + 1];
		uint32_t concat_bytes = (((uint32_t) first_bytes) << 16) | second_bytes;
		ASSERT_EQ_FMT(expected[i], concat_bytes, "%u");
		ASSERT_EQ_FMT((float) ((int32_t) expected[i]), test_buffer[i], "%f");
	}
	PASS();
}


TEST verify_i2s_dma_conversion(const struct DmaConversionLoop setup)
{
	float test_buffer[256] = { 0 };
	unsigned int output_offset = setup.input_offset / 2;
	if (setup.channel_select == R) { output_offset -= 1; }
	uint16_t first_byte = raw_i2s_dma_buffer_sine_1khz[setup.input_offset];
	uint16_t second_byte = raw_i2s_dma_buffer_sine_1khz[setup.input_offset + 1];
	uint32_t concat_bytes = (((uint32_t) first_byte) << 16) | second_byte;
	dma_i2s_halfword_to_word_complex_conversion(raw_i2s_dma_buffer_sine_1khz, test_buffer
	                                           , setup.input_bytes, setup.channel_select);
	ASSERT_EQ_FMT(setup.expected, test_buffer[output_offset], "%f"); // verify with constant
	ASSERT_EQ(test_buffer[output_offset], (float) ((int32_t) concat_bytes));
	ASSERT_EQ(test_buffer[output_offset + 1], 0.0f); // complex part == 0
	PASS();
}

void loop_dma_conversion_test(void)
{
	struct DmaConversionLoop test_setup[6] = {
		// expected output is (input offset / 2) lines offset from input array
		{ L, (int32_t) 0x00033600, 0, 4 }
		, { R, (int32_t) 0x00000000, 2, 4 }
		, { L, (int32_t) 0x00CFB180, 12, 30 }
		, { L, (int32_t) 0xFFA2BC00, 36, 38 }
		, { L, (int32_t) 0xFF1F3F80, 180, 200 }
		, { L, (int32_t) 0xFF3C8580, 444, 512 }
	};

	for (int i = 0; i < 6; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(verify_i2s_dma_conversion, test_setup[i]);
	}
}


SUITE(i2s_mic_data_processing)
{
	RUN_TEST(i2s_limits_conversion);
	// looped tests
	loop_dma_conversion_test();
}

