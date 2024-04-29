#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

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

	led_matrix_transfer_data(some_led_matrix.cs, &some_spi_reg, address, data);

	// Verify correct sequence of functions being called
	ASSERT_EQ((void*) deassert_spi_pin, fff.call_history[0]);
	ASSERT_EQ((void*) spi_tx_ready_to_transmit, fff.call_history[1]);
	ASSERT_EQ((void*) trigger_spi_transfer, fff.call_history[2]);
	ASSERT_EQ((void*) spi_tx_complete, fff.call_history[3]);
	ASSERT_EQ((void*) assert_spi_pin, fff.call_history[4]);
	PASS();
}

TEST led_matrix_set_one_bit_only(unsigned int col)
{
	struct LedMatrixTxTest led_tx = { AddrRow1, col };

	led_matrix_set_single(some_led_matrix.cs, &some_spi_reg, led_tx.address, led_tx.data);
	uint16_t tx_data = trigger_spi_transfer_fake.arg1_history[0];

	CHECK_CALL(check_led_matrix_address(tx_data, led_tx.address & 0x0F));
	CHECK_CALL(check_led_matrix_data(tx_data, 1 << col));
	PASS();
}

TEST led_matrix_set_one_rows_bits_only(unsigned int col)
{
	// previous result plus 2^nth bit
	// 0: 0000 0000
	// 1: 0000 0001
	// 2: 0000 0011 etc.
	uint8_t expected_data[9] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};
	struct LedMatrixTxTest led_tx = { AddrRow1, col };

	led_matrix_set_line_height(some_led_matrix.cs, &some_spi_reg, led_tx.address, led_tx.data);
	uint16_t tx_data = trigger_spi_transfer_fake.arg1_history[0];

	CHECK_CALL(check_led_matrix_address(tx_data, led_tx.address & 0x0F));
	CHECK_CALL(check_led_matrix_data(tx_data, expected_data[col]));
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
		RUN_TEST1(led_matrix_set_one_bit_only, i);
	}
}

void loop_test_set_bits_in_same_led_matrix_row(void)
{
	for (int i = 0; i < 9; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(led_matrix_set_one_rows_bits_only, i);
	}
}

SUITE(leds_driver)
{
	GREATEST_SET_SETUP_CB(setup_led_matrix_tests, NULL);
	RUN_TEST(led_cs_pin_set_correctly);
	RUN_TEST(led_matrix_tx_sequence);
	RUN_TEST(led_matrix_set_matrix_from_2d_array);
	// looped tests
	loop_test_led_matrix_data_input();
	loop_test_set_1_bit_in_led_matrix();
	loop_test_set_bits_in_same_led_matrix_row();
}

