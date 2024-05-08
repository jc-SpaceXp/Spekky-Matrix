#include "stm32g4_main.h"
#include "stm32g4xx_spi.h"
#include "stm32g4xx_audio.h"
#include "stm32g4xx_timers.h"
#include "dac.h"
#include "spi.h"
#include "led_matrix.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"
#include "transform_functions.h"
#include "statistics_functions.h"
#include "hw_verification/py_sine_125hz_input_test.h"


static unsigned int test_led_index = 0;

static void led_matrix_setup(void)
{
	struct LedSpiPin led_cs = { &GPIOA->BSRR, &GPIOA->ODR, SPI_CS_PIN };
	struct MaximMax2719 led_matrix;
	set_led_cs_pin_details(&led_matrix.cs, &led_cs);

	led_matrix_init(led_matrix.cs, &SPI1->DR, DATA_BRIGHTNESS_LEVEL1);
}

static void led_matrix_update_callback(xTimerHandle pxTimer)
{
	(void) pxTimer;
	test_led_index += 1;
	if (test_led_index == 8) { test_led_index = 0; }

	struct LedSpiPin led_cs = { &GPIOA->BSRR, &GPIOA->ODR, SPI_CS_PIN };
	struct MaximMax2719 led_matrix;
	set_led_cs_pin_details(&led_matrix.cs, &led_cs);

	led_matrix_set_single(led_matrix.cs, &SPI1->DR, AddrRow7, test_led_index);
	led_matrix_set_single(led_matrix.cs, &SPI1->DR, AddrRow6, (test_led_index+1) % 8);
	led_matrix_set_single(led_matrix.cs, &SPI1->DR, AddrRow5, (test_led_index+2) % 8);
	led_matrix_set_single(led_matrix.cs, &SPI1->DR, AddrRow4, (test_led_index+3) % 8);
	led_matrix_set_single(led_matrix.cs, &SPI1->DR, AddrRow3, (test_led_index+4) % 8);
	led_matrix_set_single(led_matrix.cs, &SPI1->DR, AddrRow2, (test_led_index+5) % 8);
	led_matrix_set_single(led_matrix.cs, &SPI1->DR, AddrRow1, (test_led_index+6) % 8);
	led_matrix_set_single(led_matrix.cs, &SPI1->DR, AddrRow0, (test_led_index+7) % 8);
}

// Must define when using a non-zero config for stack overflow
void vApplicationStackOverflowHook(xTaskHandle pxTask, char *pcTaskName)
{
	// remove unused warnings from compiler
	(void) pxTask;
	(void) pcTaskName;
	for (;;) {
		// do nothing
	}
}


static void dac_task(void *args __attribute((unused)))
{
	// Output to speaker
	unsigned int frequency = 1000; // Hz
	tim2_interrupt_frequency(frequency);
	struct DacTxRegisters dac_tx = { &DAC1->DHR8R1, &DAC1->DHR12R1, &DAC1->DHR12L1 };
	trigger_dac(dac_tx, 0x00, TwelveBitLeft); // base value of triangle wave
	for (;;) {
		// Task (timer is handling dac updated, nothing to do)
	}
}

static void fft_task(void *args __attribute((unused)))
{
	for (;;) {
		// cannot re-run fft as results would be overwritten
		// idle task for now
	}
}

int main (void)
{
	timer_setup(1);
	setup_hw_dac();
	setup_hw_spi();
	led_matrix_setup();

	// Run FFT before RTOS scheduler
	uint8_t inverse_fft = 0;
	uint8_t bit_reverse = 1;
	arm_cfft_instance_f32 arm_cfft;
	arm_status c_status = arm_cfft_init_1024_f32(&arm_cfft);
	(void) c_status; // suppress compiler warning
	assert_param(c_status == ARM_MATH_SUCCESS);

	arm_cfft_f32(&arm_cfft, sine32c_125hz_2048, inverse_fft, bit_reverse);
	float32_t bin_mags[1024];
	arm_cmplx_mag_f32(sine32c_125hz_2048, bin_mags, 1024);

	struct FftBinPeak {
		float32_t magnitude;
		uint32_t bin_index;
	} fft_bin_peak = { 0, 0 };
	arm_max_f32(bin_mags, 1024, &fft_bin_peak.magnitude, &fft_bin_peak.bin_index);


	BaseType_t ret_val = xTaskCreate(dac_task, "DAC out", 100, NULL, configMAX_PRIORITIES-2, NULL);
	(void) ret_val; // suppress compiler warning
	assert_param(ret_val == pdPASS);

	ret_val = xTaskCreate(fft_task, "FFT test", 100, NULL, configMAX_PRIORITIES-2, NULL);
	assert_param(ret_val == pdPASS);

	TimerHandle_t led_refresh_rate = xTimerCreate("Led matrix refresh rate"
	                                             , pdMS_TO_TICKS(200)
	                                             , pdTRUE
	                                             , NULL
	                                             , led_matrix_update_callback);
	(void) led_refresh_rate; // suppress compiler warning
	assert_param(led_refresh_rate == pdPASS);

	BaseType_t led_refresh_start = xTimerStart(led_refresh_rate, 0);
	(void) led_refresh_start; // suppress compiler warning
	assert_param(led_refresh_start == pdPASS);

	vTaskStartScheduler();


	for (;;) {
		// FreeRTOS should never let us execute this or return from main
	}

	return 0;
}
