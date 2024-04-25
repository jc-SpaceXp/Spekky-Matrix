#include "stm32g4_main.h"
#include "stm32g4xx_spi.h"
#include "stm32g4xx_audio.h"
#include "stm32g4xx_timers.h"
#include "dac.h"
#include "spi.h"
#include "led_matrix.h"

#include "FreeRTOS.h"
#include "task.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"

static void led_matrix_test(void)
{
	struct LedSpiPin led_cs = { &GPIOA->BSRR, &GPIOA->ODR, SPI_CS_PIN };
	struct MaximMax2719 led_matrix;
	set_led_cs_pin_details(&led_matrix.cs, &led_cs);

	for (int i = 0; i < 100000; ++i) {
		// lazy delay
	}

	led_matrix_init(led_matrix.cs, &SPI1->DR, DATA_INTENSITY_LEVEL1);

	for (;;) {
		for (int i = 0; i < 8; ++i) {
			for (int k = 0; k < 10000; ++k) { // lazy delay
				led_matrix_transfer_data(led_matrix.cs, &SPI1->DR, 0x07, ADDR_ROW2);
				led_matrix_transfer_data(led_matrix.cs, &SPI1->DR, 0x07, ADDR_ROW4);
				led_matrix_transfer_data(led_matrix.cs, &SPI1->DR, 1 << i, ADDR_ROW7);
			}
		}
	}
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

int main (void)
{
	timer_setup(1);
	setup_hw_dac();
	setup_hw_spi();

	led_matrix_test();

	BaseType_t ret_val = xTaskCreate(dac_task, "DAC out", 100, NULL, configMAX_PRIORITIES-1, NULL);
	(void) ret_val; // suppress compiler warning
	assert_param(ret_val == pdPASS);
	vTaskStartScheduler();

	for (;;) {
		// FreeRTOS should never let us execute this or return from main
	}

	return 0;
}
