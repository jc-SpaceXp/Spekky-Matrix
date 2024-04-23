#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "greatest.h"
#include "leds_suite.h"

#include "led_matrix.h"

static struct LedSpiPin some_cs_pin;
static struct MaximMax2719 some_led_matrix;

static uint32_t some_gpio_port_x = 0xFFFF;
static uint32_t some_gpio_port_c = 0xFFFF;


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
	unsigned int cs_pin = 2;
	set_spi_pin_details(&some_cs_pin, &some_gpio_port_c, &some_gpio_port_x, cs_pin);
	set_led_cs_pin_details(&some_led_matrix.cs, &some_cs_pin);

	ASSERT_EQ(expected_result, led_matrix_data_out(data, address));
	PASS();
}


SUITE(leds_driver)
{
	RUN_TEST(led_cs_pin_set_correctly);
	RUN_TEST(led_matrix_data_bus);
}

