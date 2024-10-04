#include "led_matrix_rtos.h"
#include "stm32g4xx_spi.h"
#include "spi.h"
#include "led_matrix.h"

#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"

#include "fft_constants.h"


extern float bin_mags[FFT_DATA_SIZE/2];
struct MaximMax2719 led_matrix;

void led_matrix_setup(int total_devices)
{
	struct LedSpiPin led_cs = { &GPIOA->BSRR, &GPIOA->ODR, SPI_CS_PIN };
	set_led_cs_pin_details(&led_matrix.cs, &led_cs);
	set_total_led_matrix_devices(&led_matrix, total_devices);

	led_matrix_init_all_quick(led_matrix, &SPI1->DR, DATA_BRIGHTNESS_LEVEL1);
}

void led_matrix_update_callback(xTimerHandle pxTimer)
{
	(void) pxTimer;

	struct LedSpiPin led_cs = { &GPIOA->BSRR, &GPIOA->ODR, SPI_CS_PIN };
	set_led_cs_pin_details(&led_matrix.cs, &led_cs);

	uint8_t bars[led_matrix.total_devices][8];
	uint8_t row_outputs[led_matrix.total_devices][8];
	for (int i = 0; i < 8; ++i) {
		for (int dev = (led_matrix.total_devices - 1); dev >= 0; --dev) {
			bars[dev][i] = fft_to_led_bar_conversion(bin_mags[i + (dev * 8)]);
		}
	}

	for (int dev = (led_matrix.total_devices - 1); dev >= 0; --dev) {
		led_matrix_convert_bars_to_rows(&bars[dev], BottomToTop, row_outputs[dev]);
	}

	for (int i = 0; i < 8; ++i) {
		// ADDR_ROW0 == 1
		for (int dev = (led_matrix.total_devices - 1); dev > 0; --dev) {
			led_matrix_transfer_data(led_matrix.cs, &SPI1->DR, i + 1, row_outputs[dev][i], NoLatchData);
		}
		led_matrix_transfer_data(led_matrix.cs, &SPI1->DR, i + 1, row_outputs[0][i], LatchData);
	}
}
