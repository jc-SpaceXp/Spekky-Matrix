#include "led_matrix_rtos.h"
#include "stm32g4xx_spi.h"
#include "spi.h"
#include "led_matrix.h"

#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"

#include "FreeRTOS.h"
#include "queue.h"

#include "fft_constants.h"

#define IC_DEVICE_ROWS 8
#define IC_DEVICE_COLS 8


extern QueueHandle_t xFftCompleteFlagQueue;
extern float db_bin_mags[FFT_DATA_SIZE/2];
struct MaximMax7219 led_matrix;

void led_matrix_setup(int total_devices)
{
	struct LedSpiPin led_cs = { &SPI_CS_PORT->BSRR, &SPI_CS_PORT->ODR, SPI_CS_PIN };
	copy_spi_pin_details(&led_matrix.cs, &led_cs);
	set_total_led_matrix_devices(&led_matrix, total_devices);

	// Pull OE to GND (all LEDs could be set)
	deassert_spi_pin(led_cs.assert_address, SPI_OE_PIN);
}

void led_matrix_update_task(void* pvParameters)
{
	(void) pvParameters;

	struct LedSpiPin led_cs = { &SPI_CS_PORT->BSRR, &SPI_CS_PORT->ODR, SPI_CS_PIN };
	copy_spi_pin_details(&led_matrix.cs, &led_cs);

	uint8_t bars[led_matrix.total_devices][IC_DEVICE_COLS];
	uint16_t row_outputs[led_matrix.total_devices][IC_DEVICE_ROWS];
	uint16_t tx_data[led_matrix.total_devices];
	int fft_complete = 0;
	for (;;) {
		while (!xQueueReceive(xFftCompleteFlagQueue, &fft_complete, portMAX_DELAY)) {
			// delay/block until data is ready
		}

		for (int i = 0; i < IC_DEVICE_COLS; ++i) {
			for (int dev = (led_matrix.total_devices - 1); dev >= 0; --dev) {
				bars[dev][i] = fft_to_led_bar_conversion(db_bin_mags[i + (dev * IC_DEVICE_COLS)]);
			}
		}

		for (int dev = (led_matrix.total_devices - 1); dev >= 0; --dev) {
			led_matrix_convert_bars_to_rows(bars[dev], IC_DEVICE_ROWS, IC_DEVICE_COLS
			                               , BottomToTop, row_outputs[dev]);
		}

		for (int i = 0; i < IC_DEVICE_ROWS; ++i) {
			// ADDR_ROW0 == 1 (therefore address == i + 1)
			for (int dev = (led_matrix.total_devices - 1); dev >= 0; --dev) {
				tx_data[dev] = max7219_led_matrix_spi_data_out(i + 1, row_outputs[dev][i]);
			}
			generic_led_matrix_transfer_data_cascade(led_matrix, &SPI1->DR, tx_data
			                                        , led_matrix.total_devices, ReverseCascade);
		}
	}
}
