#include "stm32g4xx_spi.h"
#include "reg_macros.h"

#include "stm32g4xx.h"
#include "stm32g4xx_ll_spi.h"
#include "stm32g4xx_hal_gpio.h"


static void spi_gpio_setup(void)
{
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN) | (RCC_AHB2ENR_GPIOBEN);
	// Set all to inputs
	GPIOB->MODER &= ~ (eGET_REG(GPIO_MODER_MODE, SPI_CLK_PIN)
	                  | eGET_REG(GPIO_MODER_MODE, SPI_MISO_PIN)
	                  | eGET_REG(GPIO_MODER_MODE, SPI_MOSI_PIN));
	GPIOA->MODER &= ~ eGET_REG(GPIO_MODER_MODE, SPI_CS_PIN);
	// Set GPIO (and SPI1 CS) to outputs and SPI1 to AF
	GPIOB->MODER |= eGET_REG_BIT1(GPIO_MODER_MODE, SPI_CLK_PIN)
	                | eGET_REG_BIT1(GPIO_MODER_MODE, SPI_MOSI_PIN);
	GPIOA->MODER |= eGET_REG_BIT0(GPIO_MODER_MODE, SPI_CS_PIN)
	                | eGET_REG_BIT1(GPIO_MODER_MODE, SPI_MISO_PIN);
	// Use SPI alternative function
	GPIOB->AFR[0] |= (GPIO_AF5_SPI1 << eGET_AFRL_REG(GPIO_AFRL_AFSEL, SPI_CLK_PIN))
	                 | (GPIO_AF5_SPI1 << eGET_AFRL_REG(GPIO_AFRL_AFSEL, SPI_MISO_PIN))
	                 | (GPIO_AF5_SPI1 << eGET_AFRL_REG(GPIO_AFRL_AFSEL, SPI_MOSI_PIN));
	// All GPIO ports/pins are push-pull by default (no need for OTYPER)
	// High speed pins
	GPIOB->OSPEEDR |= eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, SPI_CLK_PIN)
	                  | eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, SPI_MISO_PIN)
	                  | eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, SPI_MOSI_PIN);
	GPIOA->OSPEEDR |= eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, SPI_CS_PIN);
	// Clear reset bit on B4 (MISO, no pull-up or pull-down)
	GPIOB->PUPDR &= ~eGET_REG(GPIO_PUPDR_PUPD, SPI_MISO_PIN);
}

static void enable_spi(void)
{
	SPI1->CR1 |= SPI_CR1_SPE;
}

void setup_hw_spi(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	spi_gpio_setup();

	// Deafults: (which don't need changing)
	// Clock = Fpclk / 2 (max speed is 10MHz for led matrix, for HSI: Fpclk/2 == 8MHz)
	// CPHA 0 CPOL 0
	// MSB first
	// CRC disabled
	SPI1->CR1 |= SPI_CR1_MSTR; // STM32 is master
	SPI1->CR1 |= SPI_CR1_SSM; // Manage NSS via software
	SPI1->CR1 |= SPI_CR1_SSI; // Avoid MODEF SPI1 fault
	// LED matrix is write only (setup as simplex, master transmit only)
	SPI1->CR1 |= SPI_CR1_BIDIOE | SPI_CR1_BIDIMODE; // Write only with unidirectional data lines

	SPI1->CR2 |= LL_SPI_DATAWIDTH_16BIT;

	// Enable SPI module once setup is complete
	enable_spi();
}

bool spi_tx_ready_to_transmit(void)
{
	// TXE = 1 if any data can be sent over without an overrun
	bool is_ready = SPI1->SR & SPI_SR_TXE;
	return is_ready;
}

bool spi_tx_complete(void)
{
	// For master:
	// BSY bit is set if ongoing tx is occuring
	// (This includes if more data will be sent immediately after due to more data in TXFIFO)
	// so no need to check FTLVL
	bool spi_tx_in_progress = SPI1->SR & SPI_SR_BSY;

	return !spi_tx_in_progress;
}
