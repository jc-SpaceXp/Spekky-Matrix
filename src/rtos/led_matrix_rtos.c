#include "led_matrix_rtos.h"
#include "stm32g4xx_spi.h"
#include "spi.h"
#include "led_matrix.h"

#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"

#include "FreeRTOS.h"
#include "queue.h"

#include "fft_constants.h"

#define ROWS 8
#define COLS 32


extern QueueHandle_t xFftCompleteFlagQueue;
extern float db_bin_mags[FFT_DATA_SIZE/2];
struct Stp16cp05 led_matrix;

void led_matrix_setup(int total_devices)
{
	struct LedSpiPin led_le = { &SPI_CS_PORT->BSRR, &SPI_CS_PORT->ODR, SPI_CS_PIN };
	struct LedSpiPin led_oe = { &SPI_OE_PORT->BSRR, &SPI_OE_PORT->ODR, SPI_OE_PIN };
	copy_spi_pin_details(&led_matrix.le, &led_le);
	copy_spi_pin_details(&led_matrix.oe, &led_oe);
	set_total_stp16cp05_led_matrix_devices(&led_matrix, total_devices);

	// Pull OE to GND (all LEDs could be set, active low)
	deassert_spi_pin(led_oe.assert_address, led_oe.pin);
}

void led_matrix_update_task(void* pvParameters)
{
	(void) pvParameters;

	struct LedSpiPin led_le = { &SPI_CS_PORT->BSRR, &SPI_CS_PORT->ODR, SPI_CS_PIN };
	struct LedSpiPin led_oe = { &SPI_OE_PORT->BSRR, &SPI_OE_PORT->ODR, SPI_OE_PIN };
	copy_spi_pin_details(&led_matrix.le, &led_le);
	copy_spi_pin_details(&led_matrix.oe, &led_oe);

	uint8_t bars[COLS];
	uint32_t row_outputs[ROWS];
	uint16_t tx_data[led_matrix.total_devices];
	int fft_complete = 0;
	for (;;) {
		while (!xQueueReceive(xFftCompleteFlagQueue, &fft_complete, portMAX_DELAY)) {
			// delay/block until data is ready
		}

		for (int i = 0; i < COLS; ++i) {
			//bars[i] = fft_to_led_bar_conversion(db_bin_mags[i]);
			bars[i] = (i % 8) + 1;
		}
		bars[0] = 0; // keep 1st col empty to display an empty column

		led_matrix_bar_conversion_32bit(&bars[0], COLS, ROWS, Vertical, row_outputs);
		led_matrix_inversions_32bit(row_outputs, ROWS, DoFlipLeftRight, DoFlipVertically);

		for (int i = 0; i < ROWS; ++i) {
			set_led_matrix_device_cascade_bytes(tx_data, 0
			                                   , led_matrix_set_bit_in_row_conversion(i));
			set_led_matrix_device_cascade_bytes(tx_data, 1, (uint16_t) row_outputs[i]);
			set_led_matrix_device_cascade_bytes(tx_data, 2, (uint16_t) (row_outputs[i] >> 16u));
			generic_led_matrix_transfer_data_cascade(led_matrix.le, &SPI1->DR, tx_data
			                                        , led_matrix.total_devices, NormalCascade);

			set_led_matrix_device_cascade_bytes(tx_data, 0
			                                   , led_matrix_set_bit_in_row_conversion(i));
			// BLANK TWICE TO REDUCE GHOSTING
			// blank led to turn off transistors before switching rows
			tx_data[1] = 0x0000;
			tx_data[2] = 0x0000;
			generic_led_matrix_transfer_data_cascade(led_matrix.le, &SPI1->DR, tx_data
			                                        , led_matrix.total_devices, NormalCascade);

			// blank led to turn off transistors before switching rows
			set_led_matrix_device_cascade_bytes(tx_data, 0
			                                   , led_matrix_set_bit_in_row_conversion(i));
			generic_led_matrix_transfer_data_cascade(led_matrix.le, &SPI1->DR, tx_data
			                                        , led_matrix.total_devices, NormalCascade);
		}

	}
}
