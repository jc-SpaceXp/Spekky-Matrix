#include "led_matrix.h"
#include "spi.h"
#include "stm32g4xx_spi.h"

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

uint16_t led_matrix_data_out(uint8_t address, uint8_t data)
{
	unsigned int data_address = address & 0x0F; // address in only 4 bits wide
	return (data_address << 8) | data;
}


void led_matrix_transfer_data(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                             , uint8_t address, uint8_t data)
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

	// Pull CS high
	assert_spi_pin(cs.assert_address, cs.pin);
}

void led_matrix_clear(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg)
{
	led_matrix_transfer_data(cs, spi_tx_reg, ADDR_ROW0, 0x00);
	led_matrix_transfer_data(cs, spi_tx_reg, ADDR_ROW1, 0x00);
	led_matrix_transfer_data(cs, spi_tx_reg, ADDR_ROW2, 0x00);
	led_matrix_transfer_data(cs, spi_tx_reg, ADDR_ROW3, 0x00);
	led_matrix_transfer_data(cs, spi_tx_reg, ADDR_ROW4, 0x00);
	led_matrix_transfer_data(cs, spi_tx_reg, ADDR_ROW5, 0x00);
	led_matrix_transfer_data(cs, spi_tx_reg, ADDR_ROW6, 0x00);
	led_matrix_transfer_data(cs, spi_tx_reg, ADDR_ROW7, 0x00);
}

void led_matrix_init(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg, uint8_t brightness)
{
	led_matrix_transfer_data(cs, spi_tx_reg, ADDR_BRIGHTNESS, brightness);
	led_matrix_transfer_data(cs, spi_tx_reg, ADDR_DISPTEST, DATA_DISPTEST_OFF);
	led_matrix_transfer_data(cs, spi_tx_reg, ADDR_DECODE, DATA_DECODE_NONE);
	led_matrix_transfer_data(cs, spi_tx_reg, ADDR_SHUTDOWN, DATA_SHUTDOWN_OFF);
	// Show all 8 rows, set to 1 on startup
	led_matrix_transfer_data(cs, spi_tx_reg, ADDR_SCANLIMIT, DATA_SCANLIMIT_8_ROWS_MAX);
	led_matrix_clear(cs, spi_tx_reg);
}

void led_matrix_set_single(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                          , enum AddrRows row_addr, uint8_t col)
{
	led_matrix_transfer_data(cs, spi_tx_reg, row_addr, 1 << col);
}

void led_matrix_set_line_height(struct LedSpiPin cs, volatile uint32_t* spi_tx_reg
                               , enum AddrRows row_addr, uint8_t col_height)
{
	// 0: 0000 0000
	// 1: 0000 0001
	// 2: 0000 0011
	// 3: 0000 0111
	uint8_t output = 0;
	for (int i = 0; i < col_height; ++i) {
		output |= (1 << i);
	}
	led_matrix_transfer_data(cs, spi_tx_reg, row_addr, output);
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
		led_matrix_transfer_data(cs, spi_tx_reg, base_addr + row, output);
		output = 0;
	}
}
