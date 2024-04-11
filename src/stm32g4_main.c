#include "stm32g4_main.h"
#include "gpio_g431.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"

// Must define when using a non-zero config for stack overflow
void vApplicationStackOverflowHook(
         xTaskHandle pxTask __attribute((unused))
         , char *pcTaskName __attribute((unused)))
{
	for (;;) {
		// do nothing
	}
}


static void task1(void *args __attribute((unused)))
{
	for (;;) {
		HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

int main (void)
{
	HAL_Init();

	gpio_setup();

	xTaskCreate(task1, "Flash_LED", 100, NULL, configMAX_PRIORITIES-1, NULL);
	vTaskStartScheduler();

	for (;;) {
		// FreeRTOS should never let us execute this or return from main
	}

	return 0;
}
