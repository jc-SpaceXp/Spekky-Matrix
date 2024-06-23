#include "stm32g4xx_i2s.h"
#include "reg_macros.h"

#include "stm32g4xx.h"
#include "stm32g4xx_ll_spi.h"  // contains i2s macros
#include "stm32g4xx_hal_gpio.h"
#include "stm32g4xx_hal_rcc_ex.h"

uint32_t mic_data = 0;
int rx_ovr_error = 0;


static void i2s_gpio_setup(void)
{
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN) | (RCC_AHB2ENR_GPIOFEN);
	// Set all to inputs
	GPIOF->MODER &= ~ (eGET_REG(GPIO_MODER_MODE, I2S_CLK_PIN)
	                  | eGET_REG(GPIO_MODER_MODE, I2S_WS_PIN));
	GPIOA->MODER &= ~ (eGET_REG(GPIO_MODER_MODE, I2S_SD_PIN));
	// Set GPIO (and I2S SEL) to outputs and SPI2 to AF
	GPIOF->MODER |= eGET_REG_BIT1(GPIO_MODER_MODE, I2S_CLK_PIN)
	                | eGET_REG_BIT1(GPIO_MODER_MODE, I2S_WS_PIN);
	// Use SPI alternative function (SPI2 and I2S share same pins/registers)
	GPIOF->AFR[0] |= (GPIO_AF5_SPI2 << eGET_AFRL_REG(GPIO_AFRL_AFSEL, I2S_CLK_PIN))
	                 | (GPIO_AF5_SPI2 << eGET_AFRL_REG(GPIO_AFRL_AFSEL, I2S_WS_PIN));
	GPIOA->AFR[1] |= (GPIO_AF5_SPI2 << eGET_AFRH_REG(GPIO_AFRH_AFSEL, I2S_SD_PIN));
	// All GPIO ports/pins are push-pull by default (no need for OTYPER)
	// High speed pins
	GPIOF->OSPEEDR |= eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, I2S_CLK_PIN)
	                  | eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, I2S_WS_PIN);
	GPIOA->OSPEEDR |= eGET_REG_BIT1(GPIO_OSPEEDR_OSPEED, I2S_SD_PIN);
}

static void enable_i2s(void)
{
	SPI2->I2SCFGR |= SPI_I2SCFGR_I2SE;
}

void setup_hw_i2s(void)
{
	RCC->CCIPR |= RCC_I2SCLKSOURCE_HSI;
	RCC->APB1ENR1 |= RCC_APB1ENR1_SPI2EN;
	i2s_gpio_setup();

	// Deafults: (which don't need changing)
	// CPHA 0 CPOL 0
	// Phillips I2S mode
	// MSB first

	// Minimum clock speed needs to be > 1MHz for I2S mics, otherwise low power mode
	// Setting Fs >= 16 kHz is suitable
	// Setting clock rate to be as close to 44.1 kHz as possible, SCK = Fs * 64
	// Fs = Fi2sclk/(32(1)((2*I2SDIV)+ODD) (for no MCLK I2S setting)
	// Where Fi2sclk is equal to the APB clock for the SPI/I2S block
	// which is HCLK * APB prescaler
	// 32((2*I2SDIV)+ODD) = 16E6/44.1E3 --> 2*I2SDIV+ODD = 0.5E6/44.1E3
	// for 16MHz, ODD = 1 and I2SDIV = 5 are the closest values possible
	SPI2->I2SPR |= SPI_I2SPR_ODD | 0x05;
	SPI2->I2SCFGR |= SPI_I2SCFGR_I2SMOD; // I2S mode
	SPI2->I2SCFGR |= LL_I2S_DATAFORMAT_24B; // 24 bit data
	SPI2->I2SCFGR |= LL_I2S_MODE_MASTER_RX; // STM32 is master and receiving

	// Enable interrupts for testing, read SPI->DR to prevent overruns
	SPI2->CR2 |= SPI_CR2_RXNEIE;
	NVIC_EnableIRQ(SPI2_IRQn);
	SPI2->CR2 |= SPI_CR2_RXDMAEN;

	// Enable I2S module once setup is complete
	enable_i2s();
}

bool i2s_rx_data_in_buffer(void)
{
	bool data_available = SPI2->SR & SPI_SR_RXNE;
	return data_available;
}

bool i2s_rx_is_lchannel(void)
{
	// CHSIDE flag is unreliable if OVR flag is set
	// must ensure no overuns, must read all incoming data
	bool lchannel = !(SPI2->SR & SPI_SR_CHSIDE);
	return lchannel;
}

bool i2s_rx_is_rchannel(void)
{
	return !i2s_rx_is_lchannel();
}

bool i2s_rx_overrun(void)
{
	// Clear OVR by reading SPI2->DR and then reading SPI2->SR
	// can also detect with the ERRIE interrupt
	// still need to read with this function however to check source of fault
	return (SPI2->SR & SPI_SR_OVR);
}

void SPI2_IRQHandler(void)
{
	static bool first_byte = true;
	if (i2s_rx_data_in_buffer()) {
		uint16_t dummy_read = SPI2->DR; // read L/R data to avoid overruns
		if (i2s_rx_is_lchannel()) {
			// read_data into L channel
			mic_data |= (dummy_read >> 8);
			if (first_byte) { // MSB
				mic_data |= (dummy_read << 8);
			}
		}
		first_byte = !first_byte;
	}
	if (i2s_rx_overrun()) {
		// clear OVR flag
		rx_ovr_error = SPI2->DR;
		rx_ovr_error = SPI2->SR;
		// set global error to true
		rx_ovr_error = 1;
	}
}
