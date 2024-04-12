#include "stm32g4_main.h"
#include "stm32g4xx_audio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"

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


static void task1(void *args __attribute((unused)))
{
	for (;;) {
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

int main (void)
{
	setup_hw_dac();

	xTaskCreate(task1, "Flash_LED", 100, NULL, configMAX_PRIORITIES-1, NULL);
	vTaskStartScheduler();

	for (;;) {
		// FreeRTOS should never let us execute this or return from main
	}

	return 0;
}
