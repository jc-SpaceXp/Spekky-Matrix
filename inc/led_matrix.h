#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include "led_matrix_private.h"
#include "led_matrix_constants.h"


void set_spi_pin_details(struct LedSpiPin* spi_pin
                        , volatile uint32_t* assert_addr
                        , volatile uint32_t* deassert_addr
                        , unsigned int pin);

void set_led_cs_pin_details(struct LedSpiPin* dest, const struct LedSpiPin* src);
uint16_t led_matrix_data_out(uint8_t data, uint8_t address);
void led_matrix_transfer_data(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                             , uint8_t data, uint8_t address);

#endif /* LED_MATRIX_H */
