#ifndef STM32G4xx_TIMERS_H
#define STM32G4xx_TIMERS_H

void timer_setup(unsigned int hclk_clock_divider);
void tim2_interrupt_frequency(unsigned int hz_freq);

#endif /* STM32G4xx_TIMERS_H */
