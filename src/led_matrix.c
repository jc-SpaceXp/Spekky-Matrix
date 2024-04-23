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
