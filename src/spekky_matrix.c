#include "spekky_matrix.h"
#include "stm32g4xx_dma.h"
#include "stm32g4xx_spi.h"
#include "stm32g4xx_i2s.h"
#include "stm32g4xx_gpio_debug.h"
#include "stm32g4xx_timers.h"
#include "dac.h"
#include "spi.h"
#include "led_matrix.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "led_matrix_rtos.h"
#include "fft_rtos.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"

#include "fft_constants.h"

uint16_t i2s_dma_data[DATA_LEN] = { 0 };

QueueHandle_t xDmaFlagQueue;

int main (void)
{
	setup_hw_gpio_debug();
	setup_hw_dma(); // used for I2S
	setup_hw_i2s();


	xDmaFlagQueue = xQueueCreate(1, sizeof(int));
	(void) xDmaFlagQueue; // suppress compiler warning
	assert_param(xDmaFlagQueue == pdPASS);

	BaseType_t fft_task = xTaskCreate(fft_processing, "FFT task", 512, NULL
	                                 , configMAX_PRIORITIES - 2, NULL);
	(void) fft_task; // suppress compiler warning
	assert_param(fft_task == pdPASS);

	vTaskStartScheduler();


	for (;;) {
		// FreeRTOS should never let us execute this or return from main
	}

	return 0;
}
