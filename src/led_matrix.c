#include <limits.h>

#include "led_matrix.h"
#include "spi.h"
#include "stm32g4xx_spi.h"

/*
 * LED Matrix (MAX7219)
 *
 * TOP    (writes appear right to left visually, see example below)
 * Row 0  0000 0001 (0x01)
 * Row 1
 * Row 2
 * Row 3
 * Row 4
 * Row 5
 * Row 6
 * Row 7  1111 1111 (0xFF)
 * BOTTOM
 *
 * LED Matrix (STP16CP05)
 * Row 1 --> 0x01 to U3 (Top)
 * ...
 * Row 8 --> 0x80 to U3 (Bottom)
 *
 *       Col 32     ...       Col 1
 *  0x8000 | 0x0000 ...  0x0000 | 0x0001
 *  U5     | U4     ...  U5     | U4
 *
 *  Cascade writes: array[3] = { U3, U4, U5 } (U3 == Rows, U4 == Cols16..01, U5 == Cols32..17)
 */

const uint8_t reverse_bits_lut[256] = {
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0
	, 0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8
	, 0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4
	, 0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC
	, 0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2
	, 0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA
	, 0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6
	, 0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE
	, 0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1
	, 0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9
	, 0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5
	, 0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD
	, 0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3
	, 0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB
	, 0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7
	, 0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};


void set_spi_pin_details(struct LedSpiPin* spi_pin
                        , volatile uint32_t* assert_addr
                        , volatile uint32_t* deassert_addr
                        , unsigned int pin)
{
	*spi_pin = ((struct LedSpiPin) { assert_addr, deassert_addr, pin } );
}

void copy_spi_pin_details(struct LedSpiPin* dest, const struct LedSpiPin* src)
{
	*dest = *src;
}

void set_total_maxim_led_matrix_devices(struct MaximMax7219* matrix, int total_devices)
{
	matrix->total_devices = total_devices;
}

void set_total_stp16cp05_led_matrix_devices(struct Stp16cp05* matrix, int total_devices)
{
	matrix->total_devices = total_devices;
}

void set_led_matrix_device_cascade_bytes(uint16_t* matrix, unsigned int device_number
                                        , uint16_t tx_data)
{
	matrix[device_number] = tx_data;
}

uint16_t max7219_led_matrix_spi_data_out(uint8_t address, uint8_t data)
{
	unsigned int data_address = address & 0x0F; // address in only 4 bits wide
	return (data_address << 8) | data;
}


void led_matrix_transfer_data(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                             , uint16_t tx_data, enum LedLatchData latch)
{
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

	if (latch == LatchData) {
		// Pull CS high
		assert_spi_pin(cs.assert_address, cs.pin);
	}
}

void generic_led_matrix_transfer_data_cascade(struct LedSpiPin cs_or_le
                                             , volatile uint32_t* spi_tx_reg, uint16_t* tx_data
                                             , int total_devices
                                             , enum LedCascadeReverse reverse_order)
{
	if (reverse_order == ReverseCascade) {
		for (int i = total_devices - 1; i >= 0; --i) {
			led_matrix_transfer_data(cs_or_le, spi_tx_reg, tx_data[i], NoLatchData);
		}
	} else {
		for (int i = 0; i < total_devices; ++i) {
			led_matrix_transfer_data(cs_or_le, spi_tx_reg, tx_data[i], NoLatchData);
		}
	}

	// Latch data (pull CS high)
	assert_spi_pin(cs_or_le.assert_address, cs_or_le.pin);
}

void max7219_led_matrix_transfer_data_cascade(struct MaximMax7219 matrix
                                             , volatile uint32_t* spi_tx_reg, uint16_t tx_data
                                             , int device_number)
{
	// if X devices, must be a total of X calls to led_transfer(), one real, rest NOPs
	int initial_nops = matrix.total_devices - device_number - 1; // -1 for zero index
	uint16_t nop_data = max7219_led_matrix_spi_data_out(ADDR_NOP, 0x00);
	for (int pre = 0; pre < initial_nops; ++pre) {
		led_matrix_transfer_data(matrix.cs, spi_tx_reg, nop_data, NoLatchData);
	}

	led_matrix_transfer_data(matrix.cs, spi_tx_reg, tx_data, NoLatchData);

	for (int post = initial_nops + 1; post < matrix.total_devices; ++post) {
		led_matrix_transfer_data(matrix.cs, spi_tx_reg, nop_data, NoLatchData);
	}

	// Pull CS high
	assert_spi_pin(matrix.cs.assert_address, matrix.cs.pin);
}

void max7219_led_matrix_clear(struct MaximMax7219 matrix, volatile uint32_t* spi_tx_reg
                                , int device_number)
{
	uint16_t tx_data = 0;
	for (unsigned int i = ADDR_ROW0; i <= ADDR_ROW7; ++i) {
		tx_data = max7219_led_matrix_spi_data_out(i, 0x00);
		max7219_led_matrix_transfer_data_cascade(matrix, spi_tx_reg, tx_data, device_number);
	}
}

void max7219_led_matrix_init(struct MaximMax7219 matrix, volatile uint32_t* spi_tx_reg
                            , uint8_t brightness, int device_number)
{
	uint16_t tx_data = max7219_led_matrix_spi_data_out(ADDR_BRIGHTNESS, brightness);
	max7219_led_matrix_transfer_data_cascade(matrix, spi_tx_reg, tx_data, device_number);

	tx_data = max7219_led_matrix_spi_data_out(ADDR_DISPTEST, DATA_DISPTEST_OFF);
	max7219_led_matrix_transfer_data_cascade(matrix, spi_tx_reg, tx_data, device_number);

	tx_data = max7219_led_matrix_spi_data_out(ADDR_DECODE, DATA_DECODE_NONE);
	max7219_led_matrix_transfer_data_cascade(matrix, spi_tx_reg, tx_data, device_number);

	tx_data = max7219_led_matrix_spi_data_out(ADDR_SHUTDOWN, DATA_SHUTDOWN_OFF);
	max7219_led_matrix_transfer_data_cascade(matrix, spi_tx_reg, tx_data, device_number);

	tx_data = max7219_led_matrix_spi_data_out(ADDR_SCANLIMIT, DATA_SCANLIMIT_8_ROWS_MAX);
	max7219_led_matrix_transfer_data_cascade(matrix, spi_tx_reg, tx_data, device_number);

	max7219_led_matrix_clear(matrix, spi_tx_reg, device_number);
}

void max7219_led_matrix_init_all_quick(struct MaximMax7219 matrix, volatile uint32_t* spi_tx_reg
                                      , uint8_t brightness)
{
	// All devices must be initialised before power-up
	// We can cascade without the need of NOP to set all devices at once
	// Cannot do it 1-by-1 otherwise the 1st device is mirrored onto all others
	uint16_t tx_data = max7219_led_matrix_spi_data_out(ADDR_BRIGHTNESS, brightness);
	for (int i = 0; i < (matrix.total_devices - 1); ++i) {
		led_matrix_transfer_data(matrix.cs, spi_tx_reg, tx_data, NoLatchData);
	}
	led_matrix_transfer_data(matrix.cs, spi_tx_reg, tx_data, LatchData);

	tx_data = max7219_led_matrix_spi_data_out(ADDR_DISPTEST, DATA_DISPTEST_OFF);
	for (int i = 0; i < (matrix.total_devices - 1); ++i) {
		led_matrix_transfer_data(matrix.cs, spi_tx_reg, tx_data, NoLatchData);
	}
	led_matrix_transfer_data(matrix.cs, spi_tx_reg, tx_data, LatchData);

	tx_data = max7219_led_matrix_spi_data_out(ADDR_DECODE, DATA_DECODE_NONE);
	for (int i = 0; i < (matrix.total_devices - 1); ++i) {
		led_matrix_transfer_data(matrix.cs, spi_tx_reg, tx_data, NoLatchData);
	}
	led_matrix_transfer_data(matrix.cs, spi_tx_reg, tx_data, LatchData);

	tx_data = max7219_led_matrix_spi_data_out(ADDR_SHUTDOWN, DATA_SHUTDOWN_OFF);
	for (int i = 0; i < (matrix.total_devices - 1); ++i) {
		led_matrix_transfer_data(matrix.cs, spi_tx_reg, tx_data, NoLatchData);
	}
	led_matrix_transfer_data(matrix.cs, spi_tx_reg, tx_data, LatchData);

	tx_data = max7219_led_matrix_spi_data_out(ADDR_SCANLIMIT, DATA_SCANLIMIT_8_ROWS_MAX);
	for (int i = 0; i < (matrix.total_devices - 1); ++i) {
		led_matrix_transfer_data(matrix.cs, spi_tx_reg, tx_data, NoLatchData);
	}
	led_matrix_transfer_data(matrix.cs, spi_tx_reg, tx_data, LatchData);

	for (int i = 0; i < matrix.total_devices; ++i) {
		max7219_led_matrix_clear(matrix, spi_tx_reg, i);
	}
}

void led_matrix_set_from_2d_array(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                                 , const unsigned int (*matrix)[8][8])
{
	uint8_t base_addr = AddrRow0;
	uint8_t output = 0;
	for (int row = 0; row < 8; ++row) {
		for (int col = 0; col < 8; ++col) {
			if ((*matrix)[row][col]) {
				output |= (1 << col);
			}
		}
		uint16_t tx_data = max7219_led_matrix_spi_data_out(base_addr + row, output);
		led_matrix_transfer_data(cs, spi_tx_reg, tx_data, LatchData);
		output = 0;
	}
}

unsigned int led_matrix_set_bit_in_row_conversion(uint8_t col)
{
	return 1u << col;
}

static unsigned int led_matrix_set_line_in_row_conversion(uint8_t length)
{
	unsigned int output = 0;
	for (int i = 0; i < length; ++i) {
		output |= (1u << (i & 31));
	}
	return output;
}

void led_matrix_bar_conversion_16bit(uint8_t* col_heights
                                    , unsigned int process_rows, unsigned int process_cols
                                    , enum LedDirection direction
                                    , uint16_t* row_outputs)
{
	uint16_t output = 0;
	if (direction == Vertical) {
		// check row: ...... ....... and check if col[all_bars] exceed each row
		// e.g. if row 8 (top) we need col[bar] > 8
		for (int row = 0; row < (int) process_rows; ++row) {
			for (int b = 0; b < (int) process_cols; ++b) {
				if ((int) col_heights[b] > row) {
					output |= led_matrix_set_bit_in_row_conversion(process_cols - 1 - b);
				}
			}
			row_outputs[row] = output;
			output = 0;
		}
	} else if (direction == Horizontal) {
		for (int row = 0; row < (int) process_rows; ++row) {
			output = led_matrix_set_line_in_row_conversion(col_heights[row]);
			row_outputs[row] = output;
		}
	}
}

void led_matrix_bar_conversion_32bit(uint8_t* bar_value
                                    , unsigned int total_bars
                                    , unsigned int max_rows
                                    , enum LedDirection direction
                                    , uint32_t* row_outputs)
{
	uint32_t output = 0;
	if (direction == Vertical) {
		// check row: ...... ....... and check if col[all_bars] exceed each row
		// e.g. if row 8 (top) we need col[bar] > 8
		for (int row = 0; row < (int) max_rows; ++row) {
			for (int b = 0; b < (int) total_bars; ++b) {
				if ((int) bar_value[b] > row) {
					output |= led_matrix_set_bit_in_row_conversion(b);
				}
			}
			row_outputs[row] = output;
			output = 0;
		}
	} else if (direction == Horizontal) {
		for (int row = 0; row < (int) max_rows; ++row) {
			output = led_matrix_set_line_in_row_conversion(bar_value[row]);
			row_outputs[row] = output;
		}
	}
}

void led_matrix_inversions_16bit(uint16_t* matrix_data
                                , unsigned int max_rows
                                , enum LedHorizontalInversion horz_inversion
                                , enum LedVerticalInversion vert_inversion)
{
	uint16_t original_matrix_data[max_rows];

	if ((horz_inversion == DontFlipLeftRight) && (vert_inversion == DontFlipVertically)) {
		goto early_return;
	}

	for (int i = 0; i < (int) max_rows; ++i) {
		original_matrix_data[i] = matrix_data[i];
	}

	if (vert_inversion == DoFlipVertically) {
		for (int i = 0; i < (int) max_rows; ++i) {
			matrix_data[i] = original_matrix_data[max_rows - 1 - i];
		}
	}

	if (horz_inversion == DoFlipLeftRight) {
		for (int i = 0; i < (int) max_rows; ++i) {
			uint8_t reversed_bytes[2] = { 0 };
			reversed_bytes[0] = reverse_bits_lut[(uint8_t) matrix_data[i]];
			reversed_bytes[1] = reverse_bits_lut[(uint8_t) (matrix_data[i] >> 8)];

			matrix_data[i] = ((uint16_t) reversed_bytes[0]) << 8U;
			matrix_data[i] |= (uint16_t) reversed_bytes[1];
		}
	}

	early_return:
		return;
}

void led_matrix_inversions_32bit(uint32_t* matrix_data
                                , unsigned int max_rows
                                , enum LedHorizontalInversion horz_inversion
                                , enum LedVerticalInversion vert_inversion)
{
	uint32_t original_matrix_data[max_rows];

	if ((horz_inversion == DontFlipLeftRight) && (vert_inversion == DontFlipVertically)) {
		goto early_return;
	}

	for (int i = 0; i < (int) max_rows; ++i) {
		original_matrix_data[i] = matrix_data[i];
	}

	if (vert_inversion == DoFlipVertically) {
		for (int i = 0; i < (int) max_rows; ++i) {
			matrix_data[i] = original_matrix_data[max_rows - 1 - i];
		}
	}

	if (horz_inversion == DoFlipLeftRight) {
		for (int i = 0; i < (int) max_rows; ++i) {
			uint8_t reversed_bytes[4] = { 0 };
			reversed_bytes[0] = reverse_bits_lut[(uint8_t) matrix_data[i]];
			reversed_bytes[1] = reverse_bits_lut[(uint8_t) (matrix_data[i] >> 8)];
			reversed_bytes[2] = reverse_bits_lut[(uint8_t) (matrix_data[i] >> 16)];
			reversed_bytes[3] = reverse_bits_lut[(uint8_t) (matrix_data[i] >> 24)];

			matrix_data[i] = ((uint32_t) reversed_bytes[0]) << 24U;
			matrix_data[i] |= ((uint32_t) reversed_bytes[1]) << 16U;
			matrix_data[i] |= ((uint32_t) reversed_bytes[2]) << 8U;
			matrix_data[i] |= (uint32_t) reversed_bytes[3];
		}
	}

	early_return:
		return;
}

uint8_t fft_to_led_bar_conversion(float input_bin_mags)
{
	uint8_t bars = 0;
	int32_t db_fs = 0;
	db_fs = (int32_t) input_bin_mags;

	// scale to 0 dB FS to mic sensitivity (-25 to -27 dB FS in my case)
	if (db_fs > -4) {
		bars = 8;
	} else if (db_fs > -7) {
		bars = 7;
	} else if (db_fs > -10) {
		bars = 6;
	} else if (db_fs > -13) {
		bars = 5;
	} else if (db_fs > -15) {
		bars = 4;
	} else if (db_fs > -18) {
		bars = 3;
	} else if (db_fs > -22) {
		bars = 2;
	} else if (db_fs > -27) {
		bars = 1;
	}

	return bars;
}
