#ifndef LED_MATRIX_CONSTANTS_H
#define LED_MATRIX_CONSTANTS_H

// Address mappings
#define ADDR_NOP       0x00U
// TOP = Led Matrix writing on PCB (row0)
// BOTTOM = array on PCB (row7)
// LEFT = DIN side
// RIGHT = DOUT side, writing a 1 in rowX will place a dot in the right most position
#define ADDR_ROW0      0x01U
#define ADDR_ROW1      0x02U
#define ADDR_ROW2      0x03U
#define ADDR_ROW3      0x04U
#define ADDR_ROW4      0x05U
#define ADDR_ROW5      0x06U
#define ADDR_ROW6      0x07U
#define ADDR_ROW7      0x08U
#define ADDR_DECODE    0x09U
#define ADDR_INTENSITY 0x0AU
#define ADDR_SCANLIMIT 0x0BU
#define ADDR_SHUTDOWN  0x0CU
#define ADDR_DISPTEST  0x0FU
// Alternative namses: 7-segment names
#define ADDR_DIGIT0    ADDR_ROW0
#define ADDR_DIGIT1    ADDR_ROW1
#define ADDR_DIGIT2    ADDR_ROW2
#define ADDR_DIGIT3    ADDR_ROW3
#define ADDR_DIGIT4    ADDR_ROW4
#define ADDR_DIGIT5    ADDR_ROW5
#define ADDR_DIGIT6    ADDR_ROW6
#define ADDR_DIGIT7    ADDR_ROW7

// Data constants
#define DATA_SHUTDOWN_ON   0x00U
#define DATA_SHUTDOWN_OFF  0x01U
// Decode, set uses binary format e.g. 0b111 will set digit 7 for 7-segment display
// otherwise each bit refers to a different segment/line
#define DATA_DECODE_NONE           0x00U
#define DATA_DECODE_ALL_ROWS       0xFFU
#define DATA_DECODE_SET_ROWS(x)    (x)
#define DATA_SCANLIMIT_0_ROWS_MAX    0x00U
#define DATA_SCANLIMIT_1_ROWS_MAX    0x01U
#define DATA_SCANLIMIT_2_ROWS_MAX    0x02U
#define DATA_SCANLIMIT_3_ROWS_MAX    0x03U
#define DATA_SCANLIMIT_4_ROWS_MAX    0x04U
#define DATA_SCANLIMIT_5_ROWS_MAX    0x05U
#define DATA_SCANLIMIT_6_ROWS_MAX    0x06U
#define DATA_SCANLIMIT_7_ROWS_MAX    0x07U
#define DATA_DISPTEST_ON           0x01U
#define DATA_DISPTEST_OFF          0x00U
// Alternative names
#define DATA_DECODE_ALL_DIGITS     DATA_DECODE_ALL_ROWS
#define DATA_DECODE_SET_DIGITS(x)  DATA_DECODE_SET_ROWS(x)
#define DATA_SCANLIMIT_1_ROW_MAX   DATA_SCANLIMIT_1_ROWS_MAX
#define DATA_SCANLIMIT_0_ROWS      DATA_SCANLIMIT_0_ROWS_MAX
#define DATA_SCANLIMIT_1_ROWS      DATA_SCANLIMIT_1_ROWS_MAX
#define DATA_SCANLIMIT_2_ROWS      DATA_SCANLIMIT_2_ROWS_MAX
#define DATA_SCANLIMIT_3_ROWS      DATA_SCANLIMIT_3_ROWS_MAX
#define DATA_SCANLIMIT_4_ROWS      DATA_SCANLIMIT_4_ROWS_MAX
#define DATA_SCANLIMIT_5_ROWS      DATA_SCANLIMIT_5_ROWS_MAX
#define DATA_SCANLIMIT_6_ROWS      DATA_SCANLIMIT_6_ROWS_MAX
#define DATA_SCANLIMIT_7_ROWS      DATA_SCANLIMIT_7_ROWS_MAX

#endif /* LED_MATRIX_CONSTANTS_H */
