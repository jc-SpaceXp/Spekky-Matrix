#include "stm32g4xx_gpio_debug.h"
#include "reg_macros.h"

#include "stm32g4xx.h"
#include "stm32g4xx_hal_gpio.h"


void setup_hw_gpio_debug(void)
{
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN) | (RCC_AHB2ENR_GPIOBEN);
	// Set all to inputs
	GPIO_DEBUG_PORT->MODER &= ~eGET_REG(GPIO_MODER_MODE, GPIO_DEBUG_PIN);
	// Set GPIO to outputs
	GPIO_DEBUG_PORT->MODER |= eGET_REG_BIT0(GPIO_MODER_MODE, GPIO_DEBUG_PIN);
	// All GPIO ports/pins are push-pull by default (no need for OTYPER)
	// High speed pins
	GPIO_DEBUG_PORT->OSPEEDR |= eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, GPIO_DEBUG_PIN);
	// No pull-up or pull-down resistors
	GPIO_DEBUG_PORT->PUPDR &= ~eGET_REG(GPIO_PUPDR_PUPD, GPIO_DEBUG_PIN);
}

// Favour atomic writes rather than reusing spi assert/deassert pin code
void assert_gpio_debug_pin(void)
{
	GPIO_DEBUG_PORT->BSRR |= (1 << GPIO_DEBUG_PIN);
}

void deassert_gpio_debug_pin(void)
{
	GPIO_DEBUG_PORT->BSRR |= (1 << (GPIO_DEBUG_PIN + 16));
}
