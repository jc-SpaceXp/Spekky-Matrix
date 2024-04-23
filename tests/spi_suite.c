#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "greatest.h"
#include "spi_suite.h"

#include "spi.h"

static uint32_t some_gpio_port = 0xFFFFFFFF;
static uint32_t some_spi_tx_reg = 0xFFFF;


// Test with known value to make sure we don't rely on off-by one errors
TEST write_spi_gpio_pin_7_high(void)
{
	uint32_t init_val = 0x10000000;
	some_gpio_port = init_val;
	assert_spi_pin(&some_gpio_port, 7);
	ASSERT_EQ(some_gpio_port, (uint32_t) init_val | 0x80);
	PASS();
}

TEST write_spi_gpio_pin_7_low(void)
{
	uint32_t init_val = 0x1000FFFF;
	some_gpio_port = init_val;
	deassert_spi_pin(&some_gpio_port, 7);
	ASSERT_EQ(some_gpio_port, (uint32_t) init_val & ~0x80); // 0x1000FF7F
	PASS();
}

TEST assert_a_valid_gpio_pin_is_set_high(unsigned int pin)
{
	uint32_t init_val = 0x10000000;
	some_gpio_port = init_val;
	assert_spi_pin(&some_gpio_port, pin);
	ASSERT_EQ(some_gpio_port, (uint32_t) init_val | (1 << pin));
	PASS();
}

TEST deassert_a_valid_gpio_pin_is_set_low(unsigned int pin)
{
	uint32_t init_val = 0x1000FFFF;
	some_gpio_port = init_val;
	deassert_spi_pin(&some_gpio_port, pin);
	ASSERT_EQ(some_gpio_port, (uint32_t) init_val & ~(1 << pin));
	PASS();
}

TEST snprintf_return_val(bool sn_error)
{
	ASSERT_FALSE(sn_error);
	PASS();
}

void loop_test_assert_all_valid_gpio_pins_in_walking_ones(void)
{
	for (int i = 0; i < 16; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(assert_a_valid_gpio_pin_is_set_high, i);
	}
}

void loop_test_deassert_all_valid_gpio_pins_in_walking_zeros(void)
{
	for (int i = 0; i < 16; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(deassert_a_valid_gpio_pin_is_set_low, i);
	}
}

TEST assert_an_out_of_bound_gpio_pin_has_no_effect(unsigned int pin)
{
	uint32_t init_val = 0x00000000;
	some_gpio_port = init_val;
	assert_spi_pin(&some_gpio_port, pin);
	ASSERT_EQ(some_gpio_port, init_val);
	PASS();
}

TEST deassert_an_out_of_bound_gpio_pin_has_no_effect(unsigned int pin)
{
	uint32_t init_val = 0xFFFFFFFF;
	some_gpio_port = init_val;
	deassert_spi_pin(&some_gpio_port, pin);
	ASSERT_EQ(some_gpio_port, init_val);
	PASS();
}

void loop_test_assert_out_of_bounds_gpio_pins_in_walking_ones(void)
{
	for (int i = 16; i < 64; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(assert_an_out_of_bound_gpio_pin_has_no_effect, i);
	}
}

void loop_test_deassert_out_of_bounds_gpio_pins_in_walking_zeros(void)
{
	for (int i = 16; i < 64; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(deassert_an_out_of_bound_gpio_pin_has_no_effect, i);
	}
}

TEST data_loads_into_spi_reg_for_transfer(void)
{
	uint16_t init_val = 0xFFFF;
	some_spi_tx_reg = init_val;
	trigger_spi_transfer(&some_spi_tx_reg, 0x03);
	ASSERT_EQ(some_spi_tx_reg, 0x03);
	PASS();
}

SUITE(spi_driver)
{
	RUN_TEST(write_spi_gpio_pin_7_high);
	RUN_TEST(write_spi_gpio_pin_7_low);
	// looped test
	loop_test_assert_all_valid_gpio_pins_in_walking_ones();
	loop_test_deassert_all_valid_gpio_pins_in_walking_zeros();
	loop_test_assert_out_of_bounds_gpio_pins_in_walking_ones();
	loop_test_deassert_out_of_bounds_gpio_pins_in_walking_zeros();

	RUN_TEST(data_loads_into_spi_reg_for_transfer);
}

