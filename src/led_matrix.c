#include <limits.h>

#include "led_matrix.h"
#include "spi.h"
#include "stm32g4xx_spi.h"

/*
 * LED Matrix (MAX2719)
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

void set_led_cs_pin_details(struct LedSpiPin* dest, const struct LedSpiPin* src)
{
	*dest = *src;
}

void set_total_led_matrix_devices(struct MaximMax2719* matrix, int total_devices)
{
	matrix->total_devices = total_devices;
}

uint16_t led_matrix_data_out(uint8_t address, uint8_t data)
{
	unsigned int data_address = address & 0x0F; // address in only 4 bits wide
	return (data_address << 8) | data;
}


void led_matrix_transfer_data(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                             , uint8_t address, uint8_t data, enum LedLatchData latch)
{

	uint16_t tx_data = led_matrix_data_out(address, data);

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

void led_matrix_transfer_data_cascade(struct MaximMax2719 matrix, volatile uint32_t* spi_tx_reg
                                     , uint8_t address, uint8_t data, int device_number)
{
	// if X devices, must be a total of X calls to led_transfer(), one real, rest NOPs
	int initial_nops = matrix.total_devices - device_number - 1; // -1 for zero index
	for (int pre = 0; pre < initial_nops; ++pre) {
		led_matrix_transfer_data(matrix.cs, spi_tx_reg, ADDR_NOP, 0x00, NoLatchData);
	}

	led_matrix_transfer_data(matrix.cs, spi_tx_reg, address, data, NoLatchData);

	for (int post = initial_nops + 1; post < matrix.total_devices; ++post) {
		led_matrix_transfer_data(matrix.cs, spi_tx_reg, ADDR_NOP, 0x00, NoLatchData);
	}

	// Pull CS high
	assert_spi_pin(matrix.cs.assert_address, matrix.cs.pin);
}

void led_matrix_clear(struct MaximMax2719 matrix, volatile uint32_t* spi_tx_reg, int device_number)
{
	for (unsigned int i = ADDR_ROW0; i <= ADDR_ROW7; ++i) {
		led_matrix_transfer_data_cascade(matrix, spi_tx_reg, i, 0x00, device_number);
	}
}

void led_matrix_init(struct MaximMax2719 matrix, volatile uint32_t* spi_tx_reg
                    , uint8_t brightness, int device_number)
{
	led_matrix_transfer_data_cascade(matrix, spi_tx_reg, ADDR_BRIGHTNESS, brightness
	                                , device_number);
	led_matrix_transfer_data_cascade(matrix, spi_tx_reg, ADDR_DISPTEST, DATA_DISPTEST_OFF
	                                , device_number);
	led_matrix_transfer_data_cascade(matrix, spi_tx_reg, ADDR_DECODE, DATA_DECODE_NONE
	                                , device_number);
	led_matrix_transfer_data_cascade(matrix, spi_tx_reg, ADDR_SHUTDOWN, DATA_SHUTDOWN_OFF
	                                , device_number);
	led_matrix_transfer_data_cascade(matrix, spi_tx_reg, ADDR_SCANLIMIT
	                                , DATA_SCANLIMIT_8_ROWS_MAX, device_number);
	led_matrix_clear(matrix, spi_tx_reg, device_number);
}

void led_matrix_init_all_quick(struct MaximMax2719 matrix, volatile uint32_t* spi_tx_reg
                              , uint8_t brightness)
{
	// All devices must be initialised before power-up
	// We can cascade without the need of NOP to set all devices at once
	// Cannot do it 1-by-1 otherwise the 1st device is mirrored onto all others
	for (int i = 0; i < (matrix.total_devices - 1); ++i) {
		led_matrix_transfer_data(matrix.cs, spi_tx_reg, ADDR_BRIGHTNESS, brightness, NoLatchData);
	}
	led_matrix_transfer_data(matrix.cs, spi_tx_reg, ADDR_BRIGHTNESS, brightness, LatchData);

	for (int i = 0; i < (matrix.total_devices - 1); ++i) {
		led_matrix_transfer_data(matrix.cs, spi_tx_reg, ADDR_DISPTEST, DATA_DISPTEST_OFF
		                        , NoLatchData);
	}
	led_matrix_transfer_data(matrix.cs, spi_tx_reg, ADDR_DISPTEST, DATA_DISPTEST_OFF, LatchData);

	for (int i = 0; i < (matrix.total_devices - 1); ++i) {
		led_matrix_transfer_data(matrix.cs, spi_tx_reg, ADDR_DECODE, DATA_DECODE_NONE
		                        , NoLatchData);
	}
	led_matrix_transfer_data(matrix.cs, spi_tx_reg, ADDR_DECODE, DATA_DECODE_NONE, LatchData);

	for (int i = 0; i < (matrix.total_devices - 1); ++i) {
		led_matrix_transfer_data(matrix.cs, spi_tx_reg, ADDR_SHUTDOWN, DATA_SHUTDOWN_OFF
		                        , NoLatchData);
	}
	led_matrix_transfer_data(matrix.cs, spi_tx_reg, ADDR_SHUTDOWN, DATA_SHUTDOWN_OFF, LatchData);

	for (int i = 0; i < (matrix.total_devices - 1); ++i) {
		led_matrix_transfer_data(matrix.cs, spi_tx_reg, ADDR_SCANLIMIT, DATA_SCANLIMIT_8_ROWS_MAX
		                        , NoLatchData);
	}
	led_matrix_transfer_data(matrix.cs, spi_tx_reg, ADDR_SCANLIMIT, DATA_SCANLIMIT_8_ROWS_MAX
	                        , LatchData);

	for (int i = 0; i < matrix.total_devices; ++i) {
		led_matrix_clear(matrix, spi_tx_reg, i);
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
		led_matrix_transfer_data(cs, spi_tx_reg, base_addr + row, output, LatchData);
		output = 0;
	}
}

unsigned int led_matrix_set_bit_in_row_conversion(uint8_t col)
{
	return 1 << col;
}

static unsigned int led_matrix_set_line_in_row_conversion(uint8_t length)
{
	unsigned int output = 0;
	for (int i = 0; i < length; ++i) {
		output |= (1 << i);
	}
	return output;
}

void led_matrix_convert_bars_to_rows(uint8_t (*col_heights)[8], enum LedDirection direction
                                    , uint8_t *row_outputs)
{
	for (int row = 0; row < 8; ++row) {
		uint8_t output = led_matrix_set_line_in_row_conversion((*col_heights)[row]);

		if (direction == LeftToRight) {
			output = reverse_bits_lut[output];
		} else if (direction == TopToBottom) {
			output = 0;
			for (int bar = 0; bar < 8; ++bar) {
				// check each height exceeds the current row being checked
				// e.g. if height is equal to one then only 0th row of that bit/bar will be set
				if ((*col_heights)[bar] > row) {
					output |= (1 << (7 - bar));
				}
			}
		} else if (direction == BottomToTop) {
			output = 0;
			for (int bar = 0; bar < 8; ++bar) {
				// check each height exceeds the current (inverted) row being checked
				// e.g. if height is equal to one then only 7th row of that bit/bar will be set
				if ((*col_heights)[bar] > (7 - row)) {
					output |= (1 << (7 - bar));
				}
			}
		}
		row_outputs[row] = output;
	}
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
