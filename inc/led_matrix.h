#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include "led_matrix_private.h"
#include "led_matrix_constants.h"


void set_spi_pin_details(struct LedSpiPin* spi_pin
                        , volatile uint32_t* assert_addr
                        , volatile uint32_t* deassert_addr
                        , unsigned int pin);

void set_led_cs_pin_details(struct LedSpiPin* dest, const struct LedSpiPin* src);
uint16_t led_matrix_data_out(uint8_t address, uint8_t data);
void led_matrix_transfer_data(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                             , uint8_t address, uint8_t data);

void led_matrix_clear(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg);
void led_matrix_init(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg, uint8_t brightness);

void led_matrix_set_single(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                          , enum AddrRows row_addr, uint8_t col);
void led_matrix_set_line_height(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                               , enum AddrRows row_addr, uint8_t col_height);
void led_matrix_set_from_2d_array(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                                 , const unsigned int (*matrix)[8][8]);

#endif /* LED_MATRIX_H */
