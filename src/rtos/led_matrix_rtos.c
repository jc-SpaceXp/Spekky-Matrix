#include "led_matrix_rtos.h"
#include "stm32g4xx_spi.h"
#include "spi.h"
#include "led_matrix.h"

#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"


static unsigned int test_led_index = 0;

void led_matrix_setup(void)
{
	struct LedSpiPin led_cs = { &GPIOA->BSRR, &GPIOA->ODR, SPI_CS_PIN };
	struct MaximMax2719 led_matrix;
	set_led_cs_pin_details(&led_matrix.cs, &led_cs);

	led_matrix_init(led_matrix.cs, &SPI1->DR, DATA_BRIGHTNESS_LEVEL1);
}

void led_matrix_update_callback(xTimerHandle pxTimer)
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
