#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include "led_matrix_private.h"

void set_spi_pin_details(struct LedSpiPin* spi_pin
                        , volatile uint32_t* assert_addr
                        , volatile uint32_t* deassert_addr
                        , unsigned int pin);

void set_led_cs_pin_details(struct LedSpiPin* dest, const struct LedSpiPin* src);

#endif /* LED_MATRIX_H */
