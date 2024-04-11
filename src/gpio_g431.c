#include "gpio_g431.h"

#include "stm32g4xx_hal.h"
#include "stm32g4xx_nucleo.h"


void gpio_setup(void)
{
	LED2_GPIO_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pin = LED2_PIN;

	HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStruct);
}
