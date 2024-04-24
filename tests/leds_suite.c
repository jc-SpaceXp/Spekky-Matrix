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
	ASSERT_EQ((void*) spi_tx_ready_to_transmit, fff.call_history[1]);
	ASSERT_EQ((void*) trigger_spi_transfer, fff.call_history[2]);
	ASSERT_EQ((void*) spi_tx_complete, fff.call_history[3]);
	ASSERT_EQ((void*) assert_spi_pin, fff.call_history[4]);
	PASS();
}

#if 0
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
	struct DacTxTest dac_tx[9] = {
		{ 0x2E, EightBit, 0x2E }
		, { 0xF31A, TwelveBitRight, 0x031A}
		, { 0x0442, TwelveBitLeft,  0x4420}
		, { 0xFFFF, EightBit, 0xFF }
		, { 0x1000, TwelveBitRight, 0x0000}
		, { 0x3000, TwelveBitLeft,  0x0000}
		, { 0x1AA, EightBit, 0xAA }
		, { 0xEFFF, TwelveBitRight, 0x0FFF}
		, { 0xFFFF, TwelveBitLeft,  0xFFF0}
	};
	for (int i = 0; i < 9; ++i) {
		char test_suffix[5];
		int sn = snprintf(test_suffix, 4, "%u", i);
		bool sn_error = (sn > 5) || (sn < 0);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(snprintf_return_val, sn_error);
		greatest_set_test_suffix((const char*) &test_suffix);
		RUN_TEST1(dac_data_tx_alignment, dac_tx[i]);
	}
}
#endif

SUITE(leds_driver)
{
	GREATEST_SET_SETUP_CB(setup_led_matrix_tests, NULL);
	RUN_TEST(led_cs_pin_set_correctly);
	RUN_TEST(led_matrix_data_bus);
	RUN_TEST(led_matrix_tx_sequence);
	// looped tests
	//loop_test_dac_data_tx_alignment();
}

