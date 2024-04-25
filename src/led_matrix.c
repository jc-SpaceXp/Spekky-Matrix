#include "led_matrix.h"
#include "spi.h"
#include "stm32g4xx_spi.h"

void set_spi_pin_details(struct LedSpiPin* spi_pin
                        , volatile uint32_t* assert_addr
                        , volatile uint32_t* deassert_addr
                        , unsigned int pin)
{
	*spi_pin = ((struct LedSpiPin) { assert_addr, deassert_addr, pin } );
}

void set_led_cs_pin_details(struct LedSpiPin* dest, const struct LedSpiPin* src)
{
	*dest = *src;
}

uint16_t led_matrix_data_out(uint8_t data, uint8_t address)
{
	unsigned int data_address = address & 0x0F; // address in only 4 bits wide
	return (data_address << 8) | data;
}


void led_matrix_transfer_data(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                             , uint8_t data, uint8_t address)
{

	uint16_t tx_data = led_matrix_data_out(data, address);

	// Pull CS low
	deassert_spi_pin(cs.deassert_address, cs.pin);

	// send data
	while (!spi_tx_ready_to_transmit()) {
		// wait for spi to become free
	}

	trigger_spi_transfer(spi_tx_reg, tx_data);

	while (!spi_tx_complete()) {
		// wait for spi to finish before pulling any spi pins
	}

	// Pull CS high
	assert_spi_pin(cs.assert_address, cs.pin);
}

void led_matrix_clear(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg)
{
	led_matrix_transfer_data(cs, spi_tx_reg, 0x00, ADDR_ROW0);
	led_matrix_transfer_data(cs, spi_tx_reg, 0x00, ADDR_ROW1);
	led_matrix_transfer_data(cs, spi_tx_reg, 0x00, ADDR_ROW2);
	led_matrix_transfer_data(cs, spi_tx_reg, 0x00, ADDR_ROW3);
	led_matrix_transfer_data(cs, spi_tx_reg, 0x00, ADDR_ROW4);
	led_matrix_transfer_data(cs, spi_tx_reg, 0x00, ADDR_ROW5);
	led_matrix_transfer_data(cs, spi_tx_reg, 0x00, ADDR_ROW6);
	led_matrix_transfer_data(cs, spi_tx_reg, 0x00, ADDR_ROW7);
}

void led_matrix_init(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg, uint8_t brightness)
{
	led_matrix_transfer_data(cs, spi_tx_reg, brightness, ADDR_INTENSITY);
	led_matrix_transfer_data(cs, spi_tx_reg, DATA_DISPTEST_OFF, ADDR_DISPTEST);
	led_matrix_transfer_data(cs, spi_tx_reg, DATA_DECODE_NONE, ADDR_DECODE);
	led_matrix_transfer_data(cs, spi_tx_reg, DATA_SHUTDOWN_OFF, ADDR_SHUTDOWN);
	// Show all 8 rows, set to 1 on startup
	led_matrix_transfer_data(cs, spi_tx_reg, DATA_SCANLIMIT_8_ROWS_MAX, ADDR_SCANLIMIT);
	led_matrix_clear(cs, spi_tx_reg);
}
