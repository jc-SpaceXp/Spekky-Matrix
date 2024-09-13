#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <float.h>

#include "greatest.h"
#include "fff.h"
#include "leds_suite.h"

#include "led_matrix.h"

DEFINE_FFF_GLOBALS;
FAKE_VOID_FUNC(assert_spi_pin, volatile uint32_t*, unsigned int);
FAKE_VOID_FUNC(deassert_spi_pin, volatile uint32_t*, unsigned int);
FAKE_VOID_FUNC(trigger_spi_transfer, volatile uint32_t*, uint16_t);
FAKE_VALUE_FUNC(bool, spi_tx_ready_to_transmit);
FAKE_VALUE_FUNC(bool, spi_tx_complete);

static struct LedSpiPin some_cs_pin;
static struct MaximMax2719 some_led_matrix;

struct LedMatrixTxTest {
	uint8_t address;
	uint8_t data;
};

struct LedMatrixCascadeNopWrites {
	uint8_t data;
	uint8_t address;
	int total_devices;
	int device_write;

	uint16_t tx_data[20];

	uint8_t expected_data[20];
	uint8_t expected_addr[20];
};

static uint32_t some_gpio_port_x = 0xFFFF;
static uint32_t some_gpio_port_c = 0xFFFF;
static uint32_t some_spi_reg = 0xFFFF;


TEST check_led_matrix_data(uint16_t actual, uint8_t expected)
{
	ASSERT_EQ_FMT(expected
	             , (uint8_t) actual
	             , "%.2X");
	PASS();
}

TEST check_led_matrix_address(uint16_t actual, uint8_t expected)
{
	ASSERT_EQ_FMT(expected
	             , (uint8_t) (actual >> 8)
	             , "%.2X");
	PASS();
}

static void setup_led_matrix_tests(void* arg)
{
	unsigned int cs_pin = 9;
	set_spi_pin_details(&some_cs_pin, &some_gpio_port_c, &some_gpio_port_c, cs_pin);
	set_led_cs_pin_details(&some_led_matrix.cs, &some_cs_pin);

	RESET_FAKE(assert_spi_pin);
	RESET_FAKE(deassert_spi_pin);
	RESET_FAKE(trigger_spi_transfer);
	FFF_RESET_HISTORY();

	// Avoid inifinte loops, no spi hw so assume spi is free and tx is complete immediately
	spi_tx_ready_to_transmit_fake.return_val = true;
	spi_tx_complete_fake.return_val = true;

	(void) arg; // remove unused warning
}


TEST led_cs_pin_set_correctly(void)
{
	unsigned int cs_pin = 4;
	set_spi_pin_details(&some_cs_pin, &some_gpio_port_c, &some_gpio_port_x, cs_pin);
	set_led_cs_pin_details(&some_led_matrix.cs, &some_cs_pin);

	ASSERT_EQ(cs_pin, some_led_matrix.cs.pin);
	ASSERT_MEM_EQ(&some_gpio_port_c, some_led_matrix.cs.assert_address, 4);
	ASSERT_MEM_EQ(&some_gpio_port_x, some_led_matrix.cs.deassert_address, 4);
	PASS();
}

TEST led_matrix_data_bus(struct LedMatrixTxTest led_tx)
{
	uint16_t tx_data = led_matrix_data_out(led_tx.address, led_tx.data);

	CHECK_CALL(check_led_matrix_address(tx_data, led_tx.address & 0x0F));
	CHECK_CALL(check_led_matrix_data(tx_data, led_tx.data));
	PASS();
}

TEST led_matrix_tx_sequence(void)
{
	uint8_t data = 0xFF;
	uint8_t address = 0x21;

	led_matrix_transfer_data(some_led_matrix.cs, &some_spi_reg, address, data, LatchData);

	// Verify correct sequence of functions being called
	ASSERT_EQ((void*) deassert_spi_pin, fff.call_history[0]);
	ASSERT_EQ((void*) spi_tx_ready_to_transmit, fff.call_history[1]);
	ASSERT_EQ((void*) trigger_spi_transfer, fff.call_history[2]);
	ASSERT_EQ((void*) spi_tx_complete, fff.call_history[3]);
	ASSERT_EQ((void*) assert_spi_pin, fff.call_history[4]);
	PASS();
}

TEST led_matrix_devices_set_correctly(void)
{
	int total_devices = 8;
	set_total_led_matrix_devices(&some_led_matrix, total_devices);

	ASSERT_EQ(total_devices, some_led_matrix.total_devices);
	PASS();
}

TEST led_matrix_data_set_one_bit_only(unsigned int col)
{
	unsigned int tx_data = led_matrix_set_bit_in_row_conversion(col);

	ASSERT_EQ((int) tx_data, 1 << col);
	PASS();
}

TEST led_matrix_set_matrix_from_2d_array(void)
{
	const unsigned int input_array[8][8] = {
		{1, 0, 0, 0,     0, 0, 0, 0}
		, {0, 0, 0, 0,   0, 0, 0, 0}
		, {1, 0, 1, 0,   0, 0, 0, 0}
		, {1, 1, 1, 0,   0, 1, 0, 0}
		, {1, 1, 1, 1,   1, 1, 1, 1}
		, {1, 0, 1, 0,   0, 1, 0, 1}
		, {0, 0, 0, 1,   0, 1, 1, 1}
		, {1, 1, 1, 1,   1, 1, 1, 1}
	};
	uint8_t expected_data[8] = {0x01, 0x00, 0x05, 0x27, 0xFF, 0xA5, 0xE8, 0xFF};

	led_matrix_set_from_2d_array(some_led_matrix.cs, &some_spi_reg, &input_array);

	ASSERT_EQ(8, trigger_spi_transfer_fake.call_count);
	for (int i = 0; i < 8; ++i) {
		uint16_t tx_data = trigger_spi_transfer_fake.arg1_history[i];
		// AddrRow1 = AddrRow0 + i
		CHECK_CALL(check_led_matrix_address(tx_data, AddrRow0 + i));
		CHECK_CALL(check_led_matrix_data(tx_data, expected_data[i]));
	}
	PASS();
}


TEST snprintf_return_val(bool sn_error)
{
	ASSERT_FALSE(sn_error);
	PASS();
}

void loop_test_led_matrix_data_input(void)
{

	struct LedMatrixTxTest led_tx[3] = {
		{0xF1, 0x2F}
		, {0x01, 0x01}
		, {0x09, 0x71}
	};
	for (int i = 0; i < 3; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(led_matrix_data_bus, led_tx[i]);
	}
}

void loop_test_set_1_bit_in_led_matrix(void)
{
	for (int i = 0; i < 8; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(led_matrix_data_set_one_bit_only, i);
	}
}

TEST verify_reverse_bits_lut(void)
{
	for (int i = 0; i < 256; ++i) {
		uint8_t actual = reverse_bits_lut[i];
		uint8_t expected = 0;
		for (int b = 0; b < 8; ++b) {
			if (i & (1 << b)) {
				expected |= (1 << (7 - b));
			}
		}
		ASSERT_EQ_FMT(expected, actual, "%X");
	}
	PASS();
}


TEST led_matrix_cascade_data_calls(struct LedMatrixCascadeNopWrites* led_cascade)
{
	uint8_t data = led_cascade->data;
	uint8_t address = led_cascade->address;
	int total_devices = led_cascade->total_devices;
	set_total_led_matrix_devices(&some_led_matrix, total_devices);

	led_matrix_transfer_data_cascade(some_led_matrix, &some_spi_reg, address, data
	                                , led_cascade->device_write);
	for (int i = 0; i < total_devices; ++i) {
		led_cascade->tx_data[i] = trigger_spi_transfer_fake.arg1_history[i];
	}

	ASSERT_EQ_FMT((unsigned int) total_devices, trigger_spi_transfer_fake.call_count, "%u");
	for (int i = 0; i < total_devices; ++i) {
		CHECK_CALL(check_led_matrix_address(led_cascade->tx_data[i]
		                                   , led_cascade->expected_addr[i] & 0x0F));
		CHECK_CALL(check_led_matrix_data(led_cascade->tx_data[i], led_cascade->expected_data[i]));
	}
	PASS();
}

void loop_test_led_matrix_cascade_data(void)
{
	struct LedMatrixCascadeNopWrites test_led_cascade[4] = {
		{ 0x01, 0xEE, 1, 0, { 0 }, {0x01}, {0xEE} }
		, { 0xFF, 0x21, 2, 0, { 0 }, {0x00, 0xFF}, {ADDR_NOP, 0x21} }
		, { 0xFF, 0x21, 2, 1, { 0 }, {0xFF, 0x00}, {0x21, ADDR_NOP} }
		, { 0xF0, 0x75, 6, 2, { 0 }, {0x00, 0x00, 0x00, 0xF0, 0x00, 0x00}
		                           , {ADDR_NOP, ADDR_NOP, ADDR_NOP, 0x75, ADDR_NOP, ADDR_NOP} }
	};
	for (int i = 0; i < 4; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(led_matrix_cascade_data_calls, &test_led_cascade[i]);
	}
}

TEST led_matrix_bar_conversions(unsigned int t)
{
	struct DataAndDirection {
		uint8_t input[8];
		uint8_t output[8];
		enum LedDirection direction;
	} expected[11] = {
		{ {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
		  , {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
		  , RightToLeft }

		, { {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07}
		  , {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F}
		  , RightToLeft }

		, { {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
		  , { reverse_bits_lut[0x01]
		    , reverse_bits_lut[0x03]
		    , reverse_bits_lut[0x07]
		    , reverse_bits_lut[0x0F]
		    , reverse_bits_lut[0x1F]
		    , reverse_bits_lut[0x3F]
		    , reverse_bits_lut[0x7F]
		    , reverse_bits_lut[0xFF] }
		  , LeftToRight }

		, { {0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0xFB, 0xFF}
		  , {0x1F, 0x3F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
		  , RightToLeft }

		, { {0x08, 0xF0, 0x20, 0x88, 0x10, 0x09, 0x0A, 0x08}
		  , { reverse_bits_lut[0xFF]
		    , reverse_bits_lut[0xFF]
		    , reverse_bits_lut[0xFF]
		    , reverse_bits_lut[0xFF]
		    , reverse_bits_lut[0xFF]
		    , reverse_bits_lut[0xFF]
		    , reverse_bits_lut[0xFF]
		    , reverse_bits_lut[0xFF] }
		  , LeftToRight }

		// read as 4 down, 4 down, 4 down etc.
		// therefore first 4 rows are all filled 4 columns down
		, { {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}
		  , {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00}
		  , TopToBottom }

		// read as 5 up, 5 up, 5 up etc.
		// therefore first 4 rows are all filled 4 columns down
		, { {0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05}
		  , {0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
		  , BottomToTop }

		, { {0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08}
		  , {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
		  , BottomToTop }

		, { {0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08}
		  , {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
		  , TopToBottom }

		, { {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
		  , {0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF}
		  , BottomToTop }

		, { {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
		  , {0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01}
		  , TopToBottom }
	};

	uint8_t row_output[8] = { 0 };
	led_matrix_convert_bars_to_rows(&expected[t].input, expected[t].direction, row_output);

	ASSERT_MEM_EQ(&expected[t].output[0], row_output, 8);
	PASS();
}

void loop_test_led_matrix_bar_conversions(void)
{
	for (int i = 0; i < 11; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(led_matrix_bar_conversions, i);
	}
}

TEST led_matrix_fft_conversion(void)
{
	SKIP();
	ASSERT_EQ_FMT(0, fft_to_led_bar_conversion(0.0f), "%u"); // 0-6

	ASSERT_EQ_FMT(1, fft_to_led_bar_conversion(7.1f), "%u"); // 7-15

	ASSERT_EQ_FMT(2, fft_to_led_bar_conversion(49.8f), "%u"); // 16-255
	ASSERT_EQ_FMT(2, fft_to_led_bar_conversion(255.91f), "%u"); // 16-255

	ASSERT_EQ_FMT(3, fft_to_led_bar_conversion(256.01f), "%u"); // 256-4095
	ASSERT_EQ_FMT(3, fft_to_led_bar_conversion(4095.87f), "%u"); // 256-4095

	ASSERT_EQ_FMT(4, fft_to_led_bar_conversion(4097.22f), "%u"); // 4096-65535
	ASSERT_EQ_FMT(4, fft_to_led_bar_conversion(6555.99f), "%u"); // 4096-65535

	ASSERT_EQ_FMT(5, fft_to_led_bar_conversion(65536.43f), "%u"); // 65536-1048575
	ASSERT_EQ_FMT(5, fft_to_led_bar_conversion(1048575.63f), "%u"); // 65536-1048575

	ASSERT_EQ_FMT(6, fft_to_led_bar_conversion(1048576.66f), "%u"); // 1048575-16777215
	// fractional part tends to round up after this point
	ASSERT_EQ_FMT(16777215, (uint32_t) 16777215.41f, "%u"); // 1048575-16777215
	ASSERT_EQ_FMT(6, fft_to_led_bar_conversion(16777215.41f), "%u"); // 1048575-16777215

	ASSERT_EQ_FMT(7, fft_to_led_bar_conversion(16777216.03f), "%u"); // 16777216-268435455
	ASSERT_EQ_FMT(268435440, (uint32_t) 268435440.31f, "%u");
	ASSERT_EQ_FMT(7, fft_to_led_bar_conversion(268435440.31f), "%u"); // 16777216-268435455

	ASSERT_EQ_FMT(8, fft_to_led_bar_conversion(268435456.47f), "%u"); // 268435456-INT32_MAX
	// example of odd rounding up
	ASSERT_EQ_FMT(268435456, (uint32_t) 268435451.47f, "%u");
	ASSERT_EQ_FMT(8, fft_to_led_bar_conversion(268435454.47f), "%u"); // 268435456-INT32_MAX
	ASSERT_EQ_FMT(8, fft_to_led_bar_conversion(0x7FFFFFFFU), "%u"); // 268435456-INT32_MAX
	// INT32_MAX as signed bit is negative which always gets converted to 0's

	// Check that values above INT32_MAX are treated correctly
	ASSERT_EQ_FMT(8, fft_to_led_bar_conversion(FLT_MAX), "%u"); // 268435456-INT32_MAX

	PASS();
}

SUITE(leds_driver)
{
	GREATEST_SET_SETUP_CB(setup_led_matrix_tests, NULL);
	RUN_TEST(led_cs_pin_set_correctly);
	RUN_TEST(led_matrix_tx_sequence);
	RUN_TEST(led_matrix_devices_set_correctly);
	RUN_TEST(led_matrix_set_matrix_from_2d_array);
	RUN_TEST(verify_reverse_bits_lut);
	RUN_TEST(led_matrix_fft_conversion);
	// looped tests
	loop_test_led_matrix_data_input();
	loop_test_set_1_bit_in_led_matrix();
	loop_test_led_matrix_cascade_data();
	loop_test_led_matrix_bar_conversions();
}

