#include "stm32g4xx_audio.h"
#include "sine_1hz_dma_test.h"

#include "stm32g4xx.h"
#include "stm32g4xx_ll_dac.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_dmamux.h"
#include "stm32g4xx_hal_gpio.h"


static void dac_gpio_setup(void)
{
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	// no GPIO setup required, DAC is mapped to GPIO once DAC is enabled
	// and GPIO reset state is analogue mode anyways
}

// G4 uses DMAMUX, other devices may have a fixed dma channel for DAC1CH1
// see stm32g431xx.h for relevant structs @ bottmon of file
static void dma_setup(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_DMAMUX1EN;
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	DMAMUX1_Channel0->CCR |= LL_DMAMUX_REQ_DAC1_CH1;

	DMA1_Channel1->CCR |= DMA_CCR_CIRC; // Circular buffer
	DMA1_Channel1->CCR |= DMA_CCR_MINC; // Increment memory source addr after each access
	DMA1_Channel1->CCR |= LL_DMA_MDATAALIGN_HALFWORD; // Transfer 16-bit half-word source
	DMA1_Channel1->CCR |= LL_DMA_PDATAALIGN_HALFWORD; // Transfer 16-bit half-word dest
	DMA1_Channel1->CCR |= DMA_CCR_DIR; // Read from memory
	DMA1_Channel1->CNDTR = 1024; // Array size (memory source, sine LUT)
	DMA1_Channel1->CMAR = (uint32_t) &sine_1hz_dma_lut_test; // Memory address, DMA source (LUT)
	DMA1_Channel1->CPAR = (uint32_t) &DAC1->DHR12L1; // Peripheral address, DMA dest
}

static void enable_dma(void)
{
	// enable must occur after dma setup above
	DMA1_Channel1->CCR |= DMA_CCR_EN;
}

static void enable_dac(void)
{
	DAC1->CR |= DAC_CR_EN1;
}

void setup_hw_dac(void)
{
	RCC->AHB2ENR |= RCC_AHB2ENR_DAC1EN;
	dac_gpio_setup();
	dma_setup();
	enable_dma();

	DAC1->CR |= LL_DAC_TRIG_EXT_TIM2_TRGO;
	DAC1->CR |= DAC_CR_TEN1;
	DAC1->CR |= DAC_CR_DMAEN1;

	enable_dac();
}

bool dac_tx_ready_to_transmit(void)
{
	bool is_ready = DAC1->SR & DAC_SR_DAC1RDY;
	return is_ready;
}
