#include "stm32g4xx.h"
#include "stm32g4xx_timers.h"
#include "stm32g4xx_ll_dac.h"

void timer_setup(unsigned int hclk_clock_divider)
{
	// Clock division = 1/(PSC + 1) as counting starts from 0
	__TIM2_CLK_ENABLE();
	TIM2->CR1 |= TIM_CR1_DIR; // Down counter
	TIM2->DIER |= TIM_DIER_UIE; // Enable interrupt for TIM overflow/underflow wrap around
	TIM2->CNT = 0;
	TIM2->PSC = hclk_clock_divider - 1; // prescalers should be powers of 2
	TIM2->ARR = 0;
	NVIC_EnableIRQ(TIM2_IRQn);
}

void tim2_interrupt_frequency(unsigned int hz_freq)
{
	unsigned int us_delay_count = 15; // for a 1 us delay (minimum) 62.5ns per tick
	unsigned int freq_to_us = (1E6 / hz_freq); // us is suitable for x KHz sampling rates
	unsigned int delay_count = us_delay_count * freq_to_us;

	TIM2->CNT = delay_count;
	TIM2->ARR = delay_count;

	// Enable timer
	TIM2->CR1 |= TIM_CR1_CEN;

}

void TIM2_IRQHandler(void)
{
	// update dac, triangle wave
	DAC1->SWTRIGR |= DAC_SWTR_CH1;
	TIM2->SR &= ~TIM_SR_UIF;
}
