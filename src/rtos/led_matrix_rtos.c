#include "led_matrix_rtos.h"
#include "stm32g4xx_spi.h"
#include "spi.h"
#include "led_matrix.h"

#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"

#include "fft_constants.h"


extern float bin_mags[FFT_DATA_SIZE];

void led_matrix_setup(int total_devices)
{
	struct LedSpiPin led_cs = { &GPIOA->BSRR, &GPIOA->ODR, SPI_CS_PIN };
	struct MaximMax2719 led_matrix;
	set_led_cs_pin_details(&led_matrix.cs, &led_cs);
	set_total_led_matrix_devices(& led_matrix, total_devices);

	led_matrix_init_all_quick(led_matrix, &SPI1->DR, DATA_BRIGHTNESS_LEVEL1);
}

void led_matrix_update_callback(xTimerHandle pxTimer)
{
	(void) pxTimer;

	struct LedSpiPin led_cs = { &GPIOA->BSRR, &GPIOA->ODR, SPI_CS_PIN };
	struct MaximMax2719 led_matrix;
	set_led_cs_pin_details(&led_matrix.cs, &led_cs);

	uint8_t bars3[8] = { 0 };
	uint8_t bars2[8] = { 0 };
	uint8_t bars1[8] = { 0 };
	uint8_t bars0[8] = { 0 };
	uint8_t row_outputs3[8] = { 0 };
	uint8_t row_outputs2[8] = { 0 };
	uint8_t row_outputs1[8] = { 0 };
	uint8_t row_outputs0[8] = { 0 };
	for (int i = 0; i < 8; ++i) {
		bars3[i] = fft_to_led_bar_conversion(bin_mags[25 + i]);
		bars2[i] = fft_to_led_bar_conversion(bin_mags[17 + i]);
		bars1[i] = fft_to_led_bar_conversion(bin_mags[9 + i]);
		bars0[i] = fft_to_led_bar_conversion(bin_mags[i + 1]); // ignore DC
	}

	led_matrix_convert_bars_to_rows(&bars3, BottomToTop, row_outputs3);
	led_matrix_convert_bars_to_rows(&bars2, BottomToTop, row_outputs2);
	led_matrix_convert_bars_to_rows(&bars1, BottomToTop, row_outputs1);
	led_matrix_convert_bars_to_rows(&bars0, BottomToTop, row_outputs0);

	for (int i = 0; i < 8; ++i) {
		// ADDR_ROW0 == 1
		led_matrix_transfer_data(led_matrix.cs, &SPI1->DR, i + 1, row_outputs3[i], NoLatchData);
		led_matrix_transfer_data(led_matrix.cs, &SPI1->DR, i + 1, row_outputs2[i], NoLatchData);
		led_matrix_transfer_data(led_matrix.cs, &SPI1->DR, i + 1, row_outputs1[i], NoLatchData);
		led_matrix_transfer_data(led_matrix.cs, &SPI1->DR, i + 1, row_outputs0[i], LatchData);
	}
}
