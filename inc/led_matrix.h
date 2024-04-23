#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include "led_matrix_private.h"

// Address mappings
#define ADDR_NOP       0x00U
#define ADDR_DIGIT0    0x01U
#define ADDR_DIGIT1    0x02U
#define ADDR_DIGIT2    0x03U
#define ADDR_DIGIT3    0x04U
#define ADDR_DIGIT4    0x05U
#define ADDR_DIGIT5    0x06U
#define ADDR_DIGIT6    0x07U
#define ADDR_DIGIT7    0x08U
#define ADDR_DECODE    0x09U
#define ADDR_INTENSITY 0x0AU
#define ADDR_SCANLIMIT 0x0BU
#define ADDR_SHUTDOWN  0x0CU
#define ADDR_DISPTEST  0x0FU
// Data constants
#define DATA_SHUTDOWN_ON   0x00U
#define DATA_SHUTDOWN_OFF  0x01U
// Decode, set uses binary format e.g. 0b111 will set digit 7
// otherwise each bit refers to a different segment/line
#define DATA_DECODE_NONE        0x00U
#define DATA_DECODE_ALL_DIGITS  0xFFU
#define DATA_DECODE_SET_DIGITS(x) (x) // set bits D7-D0 to set BCD for those
// alternative name would be UPTO0, UPTO1 etc.
#define DATA_SCANLIMIT_0         0x00U
#define DATA_SCANLIMIT_01        0x01U
#define DATA_SCANLIMIT_012       0x02U
#define DATA_SCANLIMIT_0123      0x03U
#define DATA_SCANLIMIT_01234     0x04U
#define DATA_SCANLIMIT_012345    0x05U
#define DATA_SCANLIMIT_0123456   0x06U
#define DATA_SCANLIMIT_01234567  0x07U
#define DATA_DISPTEST_ON    0x01U
#define DATA_DISPTEST_OFF   0x00U


void set_spi_pin_details(struct LedSpiPin* spi_pin
                        , volatile uint32_t* assert_addr
                        , volatile uint32_t* deassert_addr
                        , unsigned int pin);

void set_led_cs_pin_details(struct LedSpiPin* dest, const struct LedSpiPin* src);
uint16_t led_matrix_data_out(uint8_t data, uint8_t address);
void led_matrix_transfer_data(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                             , uint8_t data, uint8_t address);

#endif /* LED_MATRIX_H */
