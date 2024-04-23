#include "spi.h"

enum {LAST_PIN = 15};

// Pin is zero indexed
static uint32_t pin_to_bit_pos_conversion(unsigned int pin)
{
	return (1 << pin);
}

void assert_spi_pin(volatile uint32_t* gpio_output_addr, unsigned int gpio_pin)
{
	if (gpio_pin > LAST_PIN) { return; }
	*gpio_output_addr |= pin_to_bit_pos_conversion(gpio_pin);
}

void deassert_spi_pin(volatile uint32_t* gpio_output_addr, unsigned int gpio_pin)
{
	if (gpio_pin > LAST_PIN) { return; }
	*gpio_output_addr &=  ~pin_to_bit_pos_conversion(gpio_pin);
}

void trigger_spi_transfer(volatile uint32_t* spi_tx_reg, uint16_t data)
{
	// STM32: Writes initiate TXFIFO, reads ineract w/ RXFIFO
	// STM32: Data size should be set to 16 bits for 8 bit SPI transfers
	//        unused bits are ignored on hardware
	*spi_tx_reg = data; // No checks in place so far e.g. is TXFIFO full etc.
}
