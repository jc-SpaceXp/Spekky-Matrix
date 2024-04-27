#include "stm32g4_main.h"
#include "stm32g4xx_audio.h"
#include "stm32g4xx_timers.h"
#include "dac.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"
#include "fft4cm4f.h"
#include "hw_verification/py_sine_input_test.h"


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
	}
}

int main (void)
{
	timer_setup(1);
	setup_hw_dac();


#if 0
	BaseType_t ret_val = xTaskCreate(dac_task, "DAC out", 100, NULL, configMAX_PRIORITIES-1, NULL);
	(void) ret_val; // suppress compiler warning
	assert_param(ret_val == pdPASS);

	ret_val = xTaskCreate(fft_task, "FFT test", 100, NULL, configMAX_PRIORITIES-1, NULL);
	assert_param(ret_val == pdPASS);

	vTaskStartScheduler();
#endif

	fftr4_1024(sine32f_input_fft); // result stored in array
	bool r_complete = true; // breakpoint for gdb
	fftc4_1024(sine32c_input_fft); // result stored in array
	bool c_complete = true; // breakpoint for gdb

	for (;;) {
	}

	return 0;
}
