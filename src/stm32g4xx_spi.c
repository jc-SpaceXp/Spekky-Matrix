#include "stm32g4xx_spi.h"

#include "stm32g4xx.h"
#include "stm32g4xx_ll_spi.h"
#include "stm32g4xx_hal_gpio.h"

#define GET_REG(REG, PIN) (REG ## PIN)
#define GET_REG_BIT0(REG, PIN) (REG ## PIN ## _0)
#define GET_REG_BIT1(REG, PIN) (REG ## PIN ## _1)
#define GET_AFRL_REG(REG, PIN) (REG ## PIN ## _Pos)
// Use eGET to expand the PIN macro for GET macros above
#define eGET_REG(REG, PIN) GET_REG(REG, PIN)
#define eGET_REG_BIT0(REG, PIN) GET_REG_BIT0(REG, PIN)
#define eGET_REG_BIT1(REG, PIN) GET_REG_BIT1(REG, PIN)
#define eGET_AFRL_REG(REG, PIN) GET_AFRL_REG(REG, PIN)

static void spi_gpio_setup(void)
{
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN) | (RCC_AHB2ENR_GPIOBEN);
	// Set all to inputs
	GPIOB->MODER &= ~ (eGET_REG(GPIO_MODER_MODE, SPI_CLK_PIN)
	                  | eGET_REG(GPIO_MODER_MODE, SPI_MISO_PIN)
	                  | eGET_REG(GPIO_MODER_MODE, SPI_MOSI_PIN));
	GPIOA->MODER &= ~ (eGET_REG(GPIO_MODER_MODE, SPI_CS_PIN)
	                  | eGET_REG(GPIO_MODER_MODE, GPIO_DCX_PIN)
	                  | eGET_REG(GPIO_MODER_MODE, GPIO_RSX_PIN));
	// Set GPIO (and SPI1 CS) to outputs and SPI1 to AF
	GPIOB->MODER |= eGET_REG_BIT1(GPIO_MODER_MODE, SPI_CLK_PIN)
	                | eGET_REG_BIT1(GPIO_MODER_MODE, SPI_MOSI_PIN);
	GPIOA->MODER |= eGET_REG_BIT0(GPIO_MODER_MODE, SPI_CS_PIN)
	                | eGET_REG_BIT1(GPIO_MODER_MODE, SPI_MISO_PIN)
	                | eGET_REG_BIT0(GPIO_MODER_MODE, GPIO_DCX_PIN)
	                | eGET_REG_BIT0(GPIO_MODER_MODE, GPIO_RSX_PIN);
	// Use SPI alternative function
	GPIOB->AFR[0] |= (GPIO_AF5_SPI1 << eGET_AFRL_REG(GPIO_AFRL_AFSEL, SPI_CLK_PIN))
	                 | (GPIO_AF5_SPI1 << eGET_AFRL_REG(GPIO_AFRL_AFSEL, SPI_MISO_PIN))
	                 | (GPIO_AF5_SPI1 << eGET_AFRL_REG(GPIO_AFRL_AFSEL, SPI_MOSI_PIN));
	// All GPIO ports/pins are push-pull by default (no need for OTYPER)
	// High speed pins
	GPIOB->OSPEEDR |= eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, SPI_CLK_PIN)
	                  | eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, SPI_MISO_PIN)
	                  | eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, SPI_MOSI_PIN);
	GPIOA->OSPEEDR |= eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, SPI_CS_PIN)
	                  | eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, GPIO_DCX_PIN)
	                  | eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, GPIO_RSX_PIN);
	// Clear reset bit on B4 (MISO, no pull-up or pull-down)
	GPIOB->PUPDR &= ~eGET_REG(GPIO_PUPDR_PUPD, SPI_MISO_PIN);
	// Ensure RSX is inactive immediately
	GPIOA->ODR |= eGET_REG(GPIO_ODR_OD, GPIO_RSX_PIN);
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
	// Clock = Fpclk / 2 (see below)
	// CPHA 0 CPOL 0
	// MSB first
	// CRC disabled
	SPI1->CR1 |= LL_SPI_BAUDRATEPRESCALER_DIV16; // test with a slower clock for now
	SPI1->CR1 |= SPI_CR1_MSTR; // STM32 is master
	SPI1->CR1 |= SPI_CR1_SSM; // Manage NSS via software
	SPI1->CR1 |= SPI_CR1_SSI; // Avoid MODEF SPI1 fault
	// Adafruit ST7789 display is write only according to the datasheet
	// Setup as simplex, master transmit only
	SPI1->CR1 |= SPI_CR1_BIDIOE | SPI_CR1_BIDIMODE; // Write only with unidirectional data lines

	SPI1->CR2 |= LL_SPI_DATAWIDTH_8BIT;

	// Enable SPI module once setup is complete
	enable_spi();
}

bool tx_ready_to_transmit(void)
{
	// TXE = 1 if any data can be sent over without an overrun
	bool is_ready = SPI1->SR & SPI_SR_TXE;
	return is_ready;
}

bool tx_complete(void)
{
	// For master:
	// BSY bit is set if ongoing tx is occuring
	// (This includes if more data will be sent immediately after due to more data in TXFIFO)
	// so no need to check FTLVL
	bool spi_tx_in_progress = SPI1->SR & SPI_SR_BSY;

	return !spi_tx_in_progress;
}
