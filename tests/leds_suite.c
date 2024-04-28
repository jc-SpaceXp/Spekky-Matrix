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

TEST led_matrix_data_bus(struct LedMatrixTxTest led_matrix)
{
	uint16_t expected_result = ((led_matrix.address & 0x0F) << 8) | led_matrix.data;
	ASSERT_EQ_FMT(expected_result
	             , led_matrix_data_out(led_matrix.data, led_matrix.address)
	             , "%.4X");
	PASS();
}

TEST led_matrix_tx_sequence(void)
{
	uint8_t data = 0xFF;
	uint8_t address = 0x21;

	led_matrix_transfer_data(some_led_matrix.cs, &some_spi_reg, data, address);

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
	// 1 moves into a bit pos/column for the data
	uint16_t expected_result = ((led_tx.address & 0x0F) << 8) | (1 << col);

	led_matrix_set_single(some_led_matrix.cs, &some_spi_reg, led_tx.address, led_tx.data);

	ASSERT_EQ_FMT(expected_result, trigger_spi_transfer_fake.arg1_history[0], "%4X");
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

SUITE(leds_driver)
{
	GREATEST_SET_SETUP_CB(setup_led_matrix_tests, NULL);
	RUN_TEST(led_cs_pin_set_correctly);
	RUN_TEST(led_matrix_tx_sequence);
	// looped tests
	loop_test_led_matrix_data_input();
	loop_test_set_1_bit_in_led_matrix();
}

