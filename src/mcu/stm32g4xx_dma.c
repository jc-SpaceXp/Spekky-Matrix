#include "stm32g4xx_dma.h"
#include "extern_i2s_dma_data.h"
#include "fft_constants.h"

#include "stm32g4xx.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_dmamux.h"
#include "stm32g4xx_hal_gpio.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"

extern QueueHandle_t xDmaFlagQueue;


// G4 uses DMAMUX, other devices may have a fixed dma channel for DAC1CH1
// see stm32g431xx.h for relevant structs @ bottmon of file
static void dma_i2s_setup(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_DMAMUX1EN;
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	DMAMUX1_Channel0->CCR |= LL_DMAMUX_REQ_SPI2_RX;

	DMA1_Channel1->CCR |= DMA_CCR_CIRC; // Circular buffer
	DMA1_Channel1->CCR |= DMA_CCR_MINC; // Increment memory (dest) addr after each access
	DMA1_Channel1->CCR |= LL_DMA_MDATAALIGN_HALFWORD; // Transfer 16-bit word dest
	DMA1_Channel1->CCR |= LL_DMA_PDATAALIGN_HALFWORD; // Transfer 16-bit word source
	DMA1_Channel1->CCR &= ~DMA_CCR_DIR; // Read from peripheral (I2S)
	DMA1_Channel1->CNDTR = DATA_LEN; // Array size (memory dest)
	DMA1_Channel1->CMAR = (uint32_t) &i2s_dma_data; // Memory address, DMA dest (array)
	DMA1_Channel1->CPAR = (uint32_t) &SPI2->DR; // Peripheral address, DMA source
	DMA1_Channel1->CCR |= DMA_CCR_HTIE; // Enable interrupt: Half transfer complete
	DMA1_Channel1->CCR |= DMA_CCR_TCIE; // Enable interrupt: Full transfer complete
}

static void enable_dma(void)
{
	// enable must occur after dma setup above
	DMA1_Channel1->CCR |= DMA_CCR_EN;
}

void setup_hw_dma(void)
{
	dma_i2s_setup();
	NVIC_SetPriority(DMA1_Channel1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);
	NVIC_EnableIRQ(DMA1_Channel1_IRQn);
	enable_dma();
}

void DMA1_Channel1_IRQHandler(void)
{
	int dma_flag = 0;
	if (DMA1->ISR & DMA_ISR_HTIF1) {
		DMA1->IFCR |= DMA_IFCR_CHTIF1;
		dma_flag = 1;
	} else if (DMA1->ISR & DMA_ISR_TCIF1) {
		DMA1->IFCR |= DMA_IFCR_CTCIF1;
		dma_flag = 2;
	}

	BaseType_t xHigherPriorityTask = pdFALSE;
	xQueueSendToFrontFromISR(xDmaFlagQueue, &dma_flag, &xHigherPriorityTask);
	portYIELD_FROM_ISR(xHigherPriorityTask);
}
