#include "stm32g4_main.h"
#include "stm32g4xx_audio.h"
#include "stm32g4xx_timers.h"
#include "dac.h"

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

int main (void)
{
	timer_setup(1);
	setup_hw_dac();

	xTaskCreate(dac_task, "DAC out", 100, NULL, configMAX_PRIORITIES-1, NULL);
	vTaskStartScheduler();

	for (;;) {
		// FreeRTOS should never let us execute this or return from main
	}

	return 0;
}
