#include "stm32g4xx_audio.h"

#include "stm32g4xx.h"
#include "stm32g4xx_ll_dac.h"
#include "stm32g4xx_hal_gpio.h"

static void dac_gpio_setup(void)
{
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	// no GPIO setup required, DAC is mapped to GPIO once DAC is enabled
}

static void enable_dac(void)
{
	DAC1->CR |= DAC_CR_EN1;
}

void setup_hw_dac(void)
{
	RCC->AHB2ENR |= RCC_AHB2ENR_DAC1EN;
	dac_gpio_setup();

	// Triangle wave test
	DAC1->CR |= LL_DAC_WAVE_AUTO_GENERATION_TRIANGLE;
	DAC1->CR |= LL_DAC_TRIANGLE_AMPLITUDE_511;
	DAC1->CR |= DAC_CR_TEN1;

	enable_dac();
}

bool tx_ready_to_transmit(void)
{
	bool is_ready = DAC1->SR & DAC_SR_DAC1RDY;
	return is_ready;
}
