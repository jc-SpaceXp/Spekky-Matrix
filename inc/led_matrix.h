#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include "led_matrix_private.h"
#include "led_matrix_constants.h"


extern const uint8_t reverse_bits_lut[256];

enum LedCascadeReverse { NormalCascade, ReverseCascade };
// RightToLeft is default direction of MAX7219 LED matrix
enum LedDirection { RightToLeft, LeftToRight, BottomToTop, TopToBottom };
enum LedLatchData { NoLatchData, LatchData };

void set_spi_pin_details(struct LedSpiPin* spi_pin
                        , volatile uint32_t* assert_addr
                        , volatile uint32_t* deassert_addr
                        , unsigned int pin);

void copy_spi_pin_details(struct LedSpiPin* dest, const struct LedSpiPin* src);
void set_total_maxim_led_matrix_devices(struct MaximMax7219* matrix, int total_devices);
void set_total_stp16cp05_led_matrix_devices(struct Stp16cp05* matrix, int total_devices);
uint16_t max7219_led_matrix_spi_data_out(uint8_t address, uint8_t data);
void led_matrix_transfer_data(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                             , uint16_t tx_data, enum LedLatchData latch);
void generic_led_matrix_transfer_data_cascade(struct MaximMax7219 matrix
                                             , volatile uint32_t* spi_tx_reg, uint16_t* tx_data
                                             , int total_devices
                                             , enum LedCascadeReverse reverse_order);
void max7219_led_matrix_transfer_data_cascade(struct MaximMax7219 matrix
                                             , volatile uint32_t* spi_tx_reg, uint16_t tx_data
                                             , int device_number);

void max7219_led_matrix_clear(struct MaximMax7219 matrix, volatile uint32_t* spi_tx_reg
                             , int device_number);
void max7219_led_matrix_init(struct MaximMax7219 matrix, volatile uint32_t* spi_tx_reg
                            , uint8_t brightness, int device_number);
void max7219_led_matrix_init_all_quick(struct MaximMax7219 matrix, volatile uint32_t* spi_tx_reg
                                      , uint8_t brightness);

void led_matrix_set_from_2d_array(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                                 , const unsigned int (*matrix)[8][8]);
unsigned int led_matrix_set_bit_in_row_conversion(uint8_t col);
void led_matrix_convert_bars_to_rows(uint8_t *col_height
                                    , unsigned int process_rows, unsigned int process_cols
                                    , enum LedDirection direction
                                    , uint16_t* row_outputs);
uint8_t fft_to_led_bar_conversion(float input_bin_mags);

#endif /* LED_MATRIX_H */
