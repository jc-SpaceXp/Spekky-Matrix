#include "stm32g4xx_usart.h"
#include "reg_macros.h"

#include "stm32g4xx.h"
#include "stm32g4xx_ll_usart.h"
#include "stm32g4xx_hal_gpio.h"


static void usart_gpio_setup(void)
{
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN);
	// Set all to inputs
	GPIOB->MODER &= ~ eGET_REG(GPIO_MODER_MODE, USART_TX_PIN);
	// Set TX to output
	GPIOB->MODER |= eGET_REG_BIT1(GPIO_MODER_MODE, USART_TX_PIN);
	// Use USART1 alternative function
	GPIOB->AFR[0] |= (GPIO_AF7_USART1 << eGET_AFRH_REG(GPIO_AFRH_AFSEL, USART_TX_PIN));
	// All GPIO ports/pins are push-pull by default (no need for OTYPER)
	// High speed pins
	GPIOB->OSPEEDR |= eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, USART_TX_PIN);
}

static void enable_usart(void)
{
	USART1->CR1 |= USART_CR1_UE;
}

void setup_hw_usart(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	usart_gpio_setup();

	// Deafults: (which don't need changing)
	// Parity disabled
	// 1 stop bit
	// 8 bit data (with 1 start bit and n stop bits) (M1 and M0 bits)
	// Oversampling by 16
	// FIFO disabled
	USART1->CR1 |= USART_CR1_TE; // enable USART Tx

	// Enable USART module once setup is complete
	enable_usart();
}
