#include "spekky_matrix.h"
#include "stm32g4xx_dma.h"
#include "stm32g4xx_spi.h"
#include "stm32g4xx_i2s.h"
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

uint16_t i2s_dma_data[512] = { 0 };

int main (void)
{
	setup_hw_spi();
	setup_hw_dma(); // used for I2S
	setup_hw_i2s();
	led_matrix_setup();


	TimerHandle_t led_refresh_rate = xTimerCreate("Led matrix refresh rate"
	                                             , pdMS_TO_TICKS(200)
	                                             , pdTRUE
	                                             , NULL
	                                             , led_matrix_update_callback);
	(void) led_refresh_rate; // suppress compiler warning
	assert_param(led_refresh_rate == pdPASS);

	BaseType_t led_refresh_start = xTimerStart(led_refresh_rate, 10);
	(void) led_refresh_start; // suppress compiler warning
	assert_param(led_refresh_start == pdPASS);

	TimerHandle_t fft_oneshot = xTimerCreate("Fake FFT task"
	                                        , pdMS_TO_TICKS(4)
	                                        , pdFALSE
	                                        , NULL
	                                        , fft_oneshot_callback);

	BaseType_t fft_oneshot_start = xTimerStart(fft_oneshot, 0);
	(void) fft_oneshot_start; // suppress compiler warning
	assert_param(fft_oneshot_start == pdPASS);

	vTaskStartScheduler();


	for (;;) {
		// FreeRTOS should never let us execute this or return from main
	}

	return 0;
}
