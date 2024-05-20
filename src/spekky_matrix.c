#include "spekky_matrix.h"
#include "stm32g4xx_spi.h"
#include "stm32g4xx_audio.h"
#include "stm32g4xx_timers.h"
#include "dac.h"
#include "spi.h"
#include "led_matrix.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "led_matrix_rtos.h"
#include "fft_rtos.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"


static void dac_task(void *args __attribute((unused)))
{
	// Output to speaker
	for (;;) {
		// Task (dma is handling this for now, nothing to else to do)
	}
}


int main (void)
{
	timer_setup(1);
	unsigned int frequency = 1000; // Hz
	tim2_interrupt_frequency(frequency);
	setup_hw_dac();
	setup_hw_spi();
	led_matrix_setup();

	// Run FFT before RTOS scheduler
	fake_fft_task();

	BaseType_t ret_val = xTaskCreate(dac_task, "DAC out", 100, NULL, configMAX_PRIORITIES-2, NULL);
	(void) ret_val; // suppress compiler warning
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
