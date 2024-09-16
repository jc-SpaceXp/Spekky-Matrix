#include "stm32g4xx_gpio_debug.h"
#include "reg_macros.h"

#include "stm32g4xx.h"
#include "stm32g4xx_hal_gpio.h"


void setup_hw_gpio(void)
{
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN) | (RCC_AHB2ENR_GPIOBEN);
	// Set all to inputs
	GPIOB->MODER &= ~ eGET_REG(GPIO_MODER_MODE, GPIO_DEBUG_PIN);
	// Set GPIO to outputs
	GPIOB->MODER |= eGET_REG_BIT0(GPIO_MODER_MODE, GPIO_DEBUG_PIN);
	// All GPIO ports/pins are push-pull by default (no need for OTYPER)
	// High speed pins
	GPIOB->OSPEEDR |= eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, GPIO_DEBUG_PIN);
	// Clear reset bit on B4 (MISO, no pull-up or pull-down)
	GPIOB->PUPDR &= ~eGET_REG(GPIO_PUPDR_PUPD, GPIO_DEBUG_PIN);
}
