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
static struct MaximMax7219 some_led_matrix;
static struct Stp16cp05 some_stp_led_matrix;

struct LedMatrixTxTest {
	uint8_t address;
	uint8_t data;
};

struct Max7219LedMatrixCascadeNopWrites {
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


TEST check_max7219_led_matrix_data(uint16_t actual, uint8_t expected)
{
	ASSERT_EQ_FMT(expected
	             , (uint8_t) actual
	             , "%.2X");
	PASS();
}

TEST check_max7219_led_matrix_address(uint16_t actual, uint8_t expected)
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
	copy_spi_pin_details(&some_led_matrix.cs, &some_cs_pin);

	RESET_FAKE(assert_spi_pin);
	RESET_FAKE(deassert_spi_pin);
	RESET_FAKE(trigger_spi_transfer);
	FFF_RESET_HISTORY();

	// Avoid inifinte loops, no spi hw so assume spi is free and tx is complete immediately
	spi_tx_ready_to_transmit_fake.return_val = true;
	spi_tx_complete_fake.return_val = true;

	(void) arg; // remove unused warning
}


TEST led_gpio_pins_set_correctly(void)
{
	unsigned int cs_pin = 4;
	set_spi_pin_details(&some_cs_pin, &some_gpio_port_c, &some_gpio_port_x, cs_pin);
	copy_spi_pin_details(&some_led_matrix.cs, &some_cs_pin);
	copy_spi_pin_details(&some_stp_led_matrix.le, &some_cs_pin);

	ASSERT_EQ(cs_pin, some_led_matrix.cs.pin);
	ASSERT_MEM_EQ(&some_gpio_port_c, some_led_matrix.cs.assert_address, 4);
	ASSERT_MEM_EQ(&some_gpio_port_x, some_led_matrix.cs.deassert_address, 4);
	ASSERT_EQ(cs_pin, some_stp_led_matrix.le.pin);
	ASSERT_MEM_EQ(&some_gpio_port_c, some_stp_led_matrix.le.assert_address, 4);
	ASSERT_MEM_EQ(&some_gpio_port_x, some_stp_led_matrix.le.deassert_address, 4);
	PASS();
}

TEST led_matrix_data_bus_max7219(struct LedMatrixTxTest led_tx)
{
	uint16_t tx_data = max7219_led_matrix_spi_data_out(led_tx.address, led_tx.data);

	CHECK_CALL(check_max7219_led_matrix_address(tx_data, led_tx.address & 0x0F));
	CHECK_CALL(check_max7219_led_matrix_data(tx_data, led_tx.data));
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
		CHECK_CALL(check_max7219_led_matrix_address(tx_data, AddrRow0 + i));
		CHECK_CALL(check_max7219_led_matrix_data(tx_data, expected_data[i]));
	}
	PASS();
}


TEST snprintf_return_val(bool sn_error)
{
	ASSERT_FALSE(sn_error);
	PASS();
}

TEST led_matrix_tx_sequence(enum LedLatchData latch_data)
{
	uint8_t data = 0xFF;
	uint8_t address = 0x21;
	uint16_t tx_data = max7219_led_matrix_spi_data_out(address, data);

	led_matrix_transfer_data(some_led_matrix.cs, &some_spi_reg, tx_data, latch_data);

	// Verify correct sequence of functions being called
	ASSERT_EQ((void*) deassert_spi_pin, fff.call_history[0]);
	ASSERT_EQ((void*) spi_tx_ready_to_transmit, fff.call_history[1]);
	ASSERT_EQ((void*) trigger_spi_transfer, fff.call_history[2]);
	ASSERT_EQ((void*) spi_tx_complete, fff.call_history[3]);
	if (latch_data == LatchData) {
		ASSERT_EQ(1, assert_spi_pin_fake.call_count);
		ASSERT_EQ((void*) assert_spi_pin, fff.call_history[4]);
	} else {
		ASSERT_EQ(0, assert_spi_pin_fake.call_count);
	}
	PASS();
}

void loop_led_matrix_tx_sequence(void)
{
	enum LedLatchData tx_latch[2] = { NoLatchData, LatchData };
	for (int i = 0; i < 2; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(led_matrix_tx_sequence, tx_latch[i]);
	}
}

void loop_test_max7219_led_matrix_data_input(void)
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
		RUN_TEST1(led_matrix_data_bus_max7219, led_tx[i]);
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


TEST max7219_led_matrix_cascade_data_calls(struct Max7219LedMatrixCascadeNopWrites* led_cascade)
{
	uint16_t tx_data = max7219_led_matrix_spi_data_out(led_cascade->address, led_cascade->data);
	int total_devices = led_cascade->total_devices;
	set_total_led_matrix_devices(&some_led_matrix, total_devices);

	max7219_led_matrix_transfer_data_cascade(some_led_matrix, &some_spi_reg, tx_data
	                                        , led_cascade->device_write);
	for (int i = 0; i < total_devices; ++i) {
		led_cascade->tx_data[i] = trigger_spi_transfer_fake.arg1_history[i];
	}

	ASSERT_EQ_FMT((unsigned int) total_devices, trigger_spi_transfer_fake.call_count, "%u");
	for (int i = 0; i < total_devices; ++i) {
		CHECK_CALL(check_max7219_led_matrix_address(led_cascade->tx_data[i]
		                                           , led_cascade->expected_addr[i] & 0x0F));
		CHECK_CALL(check_max7219_led_matrix_data(led_cascade->tx_data[i]
		                                           , led_cascade->expected_data[i]));
	}
	PASS();
}

void loop_test_max7219_led_matrix_cascade_data(void)
{
	struct Max7219LedMatrixCascadeNopWrites test_led_cascade[4] = {
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
		RUN_TEST1(max7219_led_matrix_cascade_data_calls, &test_led_cascade[i]);
	}
}

TEST generic_led_matrix_cascade_data_calls(enum LedCascadeReverse reverse_order)
{
	int total_devices = 2;
	uint16_t tx_data[2] = { 0x0001, 0xFF00 };

	generic_led_matrix_transfer_data_cascade(some_led_matrix, &some_spi_reg, &tx_data[0]
	                                        , total_devices, reverse_order);

	ASSERT_EQ_FMT((unsigned int) total_devices, trigger_spi_transfer_fake.call_count, "%u");
	if (reverse_order == ReverseCascade) {
		ASSERT_EQ_FMT((uint16_t) tx_data[0], trigger_spi_transfer_fake.arg1_history[1], "%.4X");
		ASSERT_EQ_FMT((uint16_t) tx_data[1], trigger_spi_transfer_fake.arg1_history[0], "%.4X");
	} else {
		ASSERT_EQ_FMT((uint16_t) tx_data[0], trigger_spi_transfer_fake.arg1_history[0], "%.4X");
		ASSERT_EQ_FMT((uint16_t) tx_data[1], trigger_spi_transfer_fake.arg1_history[1], "%.4X");
	}
	PASS();
}

void loop_test_generic_led_matrix_cascade_data(void)
{
	enum LedCascadeReverse cascade_order[2] = { NormalCascade, ReverseCascade };
	for (int i = 0; i < 2; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(generic_led_matrix_cascade_data_calls, cascade_order[i]);
	}
}

TEST led_matrix_bar_conversions_8_rows_variations(unsigned int t)
{
	unsigned int rows = 8;
	unsigned int cols = 8; // only used in Top/Bottom enums, Left/Right test 16 pixel width
	struct DataAndDirection {
		uint8_t input[8]; // 8 rows or cols depending on enum
		uint16_t output[8]; // 8x16 matrix
		enum LedDirection direction;
	} expected[13] = {
		{ {0x00,       0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00}
		  , {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}
		  , RightToLeft }

		, { {  0x00,   0x01,   0x02,   0x03,   0x04,   0x05,   0x06,   0x07}
		  , {0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F}
		  , RightToLeft }

		, { {  0x08,   0x09,   0x0A,   0x0B,   0x0C,   0x0D,   0x0E,   0x0F}
		  , {0x00FF, 0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF}
		  , RightToLeft }

		, { {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}
		  , { reverse_bits_lut[0x01] << 8
		    , reverse_bits_lut[0x03] << 8
		    , reverse_bits_lut[0x07] << 8
		    , reverse_bits_lut[0x0F] << 8
		    , reverse_bits_lut[0x1F] << 8
		    , reverse_bits_lut[0x3F] << 8
		    , reverse_bits_lut[0x7F] << 8
		    , reverse_bits_lut[0xFF] << 8 }
		  , LeftToRight }


		, { {0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}
		  , { 0xFF00 // 8 = 0x00FF (8 bits set)
		    , 0xFF00 | reverse_bits_lut[0x01]
		    , 0xFF00 | reverse_bits_lut[0x03]
		    , 0xFF00 | reverse_bits_lut[0x07]
		    , 0xFF00 | reverse_bits_lut[0x0F]
		    , 0xFF00 | reverse_bits_lut[0x1F]
		    , 0xFF00 | reverse_bits_lut[0x3F]
		    , 0xFF00 | reverse_bits_lut[0x7F] }
		  , LeftToRight }

		, { {  0x0F,   0x1F,   0x2A,   0x3B,   0x4C,   0x5D,   0x6E,   0xFF}
		  , {0x7FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF}
		  , RightToLeft }

		, { {0x08, 0xF0, 0x20, 0x88, 0x10, 0x39, 0x4A, 0xFF}
		  , { 0xFF00 // 8 = 0x00FF (8 bits set)
		    , 0xFF00 | reverse_bits_lut[0xFF]
		    , 0xFF00 | reverse_bits_lut[0xFF]
		    , 0xFF00 | reverse_bits_lut[0xFF]
		    , 0xFF00 | reverse_bits_lut[0xFF]
		    , 0xFF00 | reverse_bits_lut[0xFF]
		    , 0xFF00 | reverse_bits_lut[0xFF]
		    , 0xFF00 | reverse_bits_lut[0xFF] }
		  , LeftToRight }

		// TopToBottom and BottomToTop examples are limited to 8 cols
		// due to the input[8]

		// read as 4 down, 4 down, 4 down etc. (from right to left)
		// therefore first 4 rows are all filled 4 columns down
		, { {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}
		  , {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00}
		  , TopToBottom }

		// read as 5 up, 5 up, 5 up etc. (from right to left)
		// therefore last 5 rows are all filled 5 columns up
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

	uint16_t row_output[8] = { 0 };
	led_matrix_convert_bars_to_rows(expected[t].input, rows, cols
	                               , expected[t].direction, row_output);

	ASSERT_MEM_EQ(&expected[t].output[0], row_output, 16);
	PASS();
}

void loop_test_led_matrix_bar_conversions_8_rows(void)
{
	for (int i = 0; i < 13; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(led_matrix_bar_conversions_8_rows_variations, i);
	}
}

TEST led_matrix_bar_conversions_16_rows_variations(unsigned int t)
{
	unsigned int rows = 16;
	unsigned int cols = 16;
	struct DataAndDirection {
		uint8_t input[16]; // 16 rows or cols depending on enum
		uint16_t output[16]; // 16x16 matrix
		enum LedDirection direction;
	} expected[10] = {
		// TopToBottom and BottomToTop examples are limited to 16 cols
		// due to the input[16]

		// read as 4 down, 4 down, 4 down etc. (from right to left)
		// therefore first 4 rows are all filled 4 columns down
		{ {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04
		    , 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 }
		  , {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x00, 0x00, 0x00, 0x00
		    , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
		  , TopToBottom }

		// read as 5 up, 5 up, 5 up etc. (from right to left)
		// therefore last 5 rows are all filled 5 columns up
		, { {0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05
		    , 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05 }
		  , {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		    , 0x00, 0x00, 0x00, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF }
		  , BottomToTop }

		, { {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F
		    , 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F }
		  , {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
		    , 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF }
		  , BottomToTop }

		, { {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F
		    , 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F }
		  , {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
		    , 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF }
		  , TopToBottom }

		, { {0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
		    , 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08 }
		  , {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		    , 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF }
		  , BottomToTop }

		, { {0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
		    , 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08 }
		  , {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
		    , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
		  , TopToBottom }

		, { {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
		    , 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x1F }
		  , {0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF
		    , 0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF }
		  , BottomToTop }

		, { {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
		    , 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x1F }
		  , {0xFFFF, 0x7FFF, 0x3FFF, 0x1FFF, 0x0FFF, 0x07FF, 0x03FF, 0x01FF
		    , 0x00FF, 0x007F, 0x003F, 0x001F, 0x000F, 0x0007, 0x0003, 0x0001 }
		  , TopToBottom }

		, { {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
		    , 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x1F }
		  , {0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF
		    , 0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF }
		  , RightToLeft }

		, { {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
		    , 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x1F }
		  , { reverse_bits_lut[0x01] << 8
		    , reverse_bits_lut[0x03] << 8
		    , reverse_bits_lut[0x07] << 8
		    , reverse_bits_lut[0x0F] << 8
		    , reverse_bits_lut[0x1F] << 8
		    , reverse_bits_lut[0x3F] << 8
		    , reverse_bits_lut[0x7F] << 8
		    , reverse_bits_lut[0xFF] << 8
			// more than 8 bits set, therefore 0xFF00 is always set
		    , 0xFF00 | reverse_bits_lut[0x01]
		    , 0xFF00 | reverse_bits_lut[0x03]
		    , 0xFF00 | reverse_bits_lut[0x07]
		    , 0xFF00 | reverse_bits_lut[0x0F]
		    , 0xFF00 | reverse_bits_lut[0x1F]
		    , 0xFF00 | reverse_bits_lut[0x3F]
		    , 0xFF00 | reverse_bits_lut[0x7F]
		    , 0xFF00 | reverse_bits_lut[0xFF] }
		  , LeftToRight }
	};

	uint16_t row_output[16] = { 0 };
	led_matrix_convert_bars_to_rows(expected[t].input, rows, cols
	                               , expected[t].direction, row_output);

	ASSERT_MEM_EQ(&expected[t].output[0], row_output, 32);
	PASS();
}

void loop_test_led_matrix_bar_conversions_16_rows(void)
{
	for (int i = 0; i < 10; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(led_matrix_bar_conversions_16_rows_variations, i);
	}
}

TEST led_matrix_fft_conversion(void)
{
	ASSERT_EQ_FMT(8, fft_to_led_bar_conversion(-3.99f), "%u"); // above -4 dB FS
	ASSERT_EQ_FMT(8, fft_to_led_bar_conversion(-0.20f), "%u"); // above -4 dB FS
	ASSERT_EQ_FMT(8, fft_to_led_bar_conversion(10.00f), "%u"); // above -4 dB FS (out of range +ve)

	ASSERT_EQ_FMT(7, fft_to_led_bar_conversion(-6.99f), "%u"); // above -7 dB FS
	ASSERT_EQ_FMT(7, fft_to_led_bar_conversion(-4.00f), "%u"); // above -7 dB FS
	ASSERT_EQ_FMT(7, fft_to_led_bar_conversion(-5.20f), "%u"); // above -7 dB FS

	ASSERT_EQ_FMT(6, fft_to_led_bar_conversion(-9.99f), "%u"); // above -10 dB FS
	ASSERT_EQ_FMT(6, fft_to_led_bar_conversion(-7.00f), "%u"); // above -10 dB FS
	ASSERT_EQ_FMT(6, fft_to_led_bar_conversion(-8.41f), "%u"); // above -10 dB FS

	ASSERT_EQ_FMT(5, fft_to_led_bar_conversion(-12.99f), "%u"); // above -13 dB FS
	ASSERT_EQ_FMT(5, fft_to_led_bar_conversion(-10.00f), "%u"); // above -13 dB FS
	ASSERT_EQ_FMT(5, fft_to_led_bar_conversion(-11.11f), "%u"); // above -13 dB FS

	ASSERT_EQ_FMT(4, fft_to_led_bar_conversion(-14.99f), "%u"); // above -15 dB FS
	ASSERT_EQ_FMT(4, fft_to_led_bar_conversion(-13.00f), "%u"); // above -15 dB FS
	ASSERT_EQ_FMT(4, fft_to_led_bar_conversion(-14.22f), "%u"); // above -15 dB FS

	ASSERT_EQ_FMT(3, fft_to_led_bar_conversion(-17.99f), "%u"); // above -18 dB FS
	ASSERT_EQ_FMT(3, fft_to_led_bar_conversion(-15.00f), "%u"); // above -18 dB FS
	ASSERT_EQ_FMT(3, fft_to_led_bar_conversion(-16.56f), "%u"); // above -18 dB FS

	ASSERT_EQ_FMT(2, fft_to_led_bar_conversion(-21.99f), "%u"); // above -22 dB FS
	ASSERT_EQ_FMT(2, fft_to_led_bar_conversion(-18.00f), "%u"); // above -22 dB FS
	ASSERT_EQ_FMT(2, fft_to_led_bar_conversion(-20.87f), "%u"); // above -22 dB FS

	ASSERT_EQ_FMT(1, fft_to_led_bar_conversion(-26.99f), "%u"); // above -27 dB FS
	ASSERT_EQ_FMT(1, fft_to_led_bar_conversion(-22.00f), "%u"); // above -27 dB FS
	ASSERT_EQ_FMT(1, fft_to_led_bar_conversion(-25.50f), "%u"); // above -27 dB FS

	ASSERT_EQ_FMT(0, fft_to_led_bar_conversion(-27.00f), "%u"); // below -27 dB FS
	ASSERT_EQ_FMT(0, fft_to_led_bar_conversion(-40.00f), "%u"); // below -27 dB FS
	ASSERT_EQ_FMT(0, fft_to_led_bar_conversion(-156.99f), "%u"); // below -27 dB FS
	ASSERT_EQ_FMT(0, fft_to_led_bar_conversion(-14341456.99f), "%u"); // below -27 dB FS

	PASS();
}

SUITE(leds_driver)
{
	GREATEST_SET_SETUP_CB(setup_led_matrix_tests, NULL);
	RUN_TEST(led_gpio_pins_set_correctly);
	RUN_TEST(led_matrix_devices_set_correctly);
	RUN_TEST(led_matrix_set_matrix_from_2d_array);
	RUN_TEST(verify_reverse_bits_lut);
	RUN_TEST(led_matrix_fft_conversion);
	// looped tests
	loop_led_matrix_tx_sequence();
	loop_test_max7219_led_matrix_data_input();
	loop_test_set_1_bit_in_led_matrix();
	loop_test_max7219_led_matrix_cascade_data();
	loop_test_generic_led_matrix_cascade_data();
	loop_test_led_matrix_bar_conversions_8_rows();
	loop_test_led_matrix_bar_conversions_16_rows();
}

