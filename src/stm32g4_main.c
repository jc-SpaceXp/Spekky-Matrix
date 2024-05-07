#include "stm32g4_main.h"
#include "stm32g4xx_audio.h"
#include "stm32g4xx_timers.h"
#include "dac.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"
#include "transform_functions.h"
#include "statistics_functions.h"
#include "hw_verification/py_sine_125hz_input_test.h"


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

	uint8_t inverse_fft = 0;
	uint8_t bit_reverse = 1;
	arm_cfft_instance_f32 arm_cfft;
	arm_status c_status = arm_cfft_init_1024_f32(&arm_cfft);
	arm_cfft_f32(&arm_cfft, sine32c_125hz_2048, inverse_fft, bit_reverse);
	float32_t bin_mags[1024];
	arm_cmplx_mag_f32(sine32c_125hz_2048, bin_mags, 1024);

	struct FftBinPeak {
		float32_t magnitude;
		uint32_t bin_index;
	} fft_bin_peak = { 0, 0 };
	arm_max_f32(bin_mags, 1024, &fft_bin_peak.magnitude, &fft_bin_peak.bin_index);

	bool c_complete = true; // breakpoint for gdb

	for (;;) {
	}

	return 0;
}
