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

static struct LedSpiPin some_cs_pin;
static struct MaximMax2719 some_led_matrix;

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

TEST led_matrix_data_bus(void)
{
	uint8_t data = 0xF1;
	uint8_t address = 0x2F;
	uint16_t expected_result = 0x0FF1;

	ASSERT_EQ(expected_result, led_matrix_data_out(data, address));
	PASS();
}

TEST led_matrix_tx_sequence(void)
{
	uint8_t data = 0xFF;
	uint8_t address = 0x21;

	led_matrix_transfer_data(some_led_matrix.cs, &some_spi_reg, data, address);

	// Verify correct sequence of functions being called
	ASSERT_EQ((void*) deassert_spi_pin, fff.call_history[0]);
	ASSERT_EQ((void*) trigger_spi_transfer, fff.call_history[1]);
	ASSERT_EQ((void*) assert_spi_pin, fff.call_history[2]);
	PASS();
}


SUITE(leds_driver)
{
	GREATEST_SET_SETUP_CB(setup_led_matrix_tests, NULL);
	RUN_TEST(led_cs_pin_set_correctly);
	RUN_TEST(led_matrix_data_bus);
	RUN_TEST(led_matrix_tx_sequence);
}

