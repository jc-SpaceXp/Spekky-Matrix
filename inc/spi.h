#ifndef SPI_H
#define SPI_H

#include <stdint.h>

void assert_spi_pin(volatile uint32_t* gpio_output_addr, unsigned int gpio_pin);
void deassert_spi_pin(volatile uint32_t* gpio_output_addr, unsigned int gpio_pin);

void trigger_spi_transfer(volatile uint32_t* spi_tx_reg, uint16_t data);

#endif /* SPI_H */
