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

int main (void)
{
	timer_setup(1);
	setup_hw_dac();
	setup_hw_spi();

	led_matrix_setup();

	BaseType_t ret_val = xTaskCreate(dac_task, "DAC out", 100, NULL, configMAX_PRIORITIES-1, NULL);
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
