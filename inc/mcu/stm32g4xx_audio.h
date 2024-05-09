#ifndef STM32G4xx_ADUIO_H
#define STM32G4xx_AUDIO_H

#include <stdbool.h>

void setup_hw_dac(void);
bool dac_tx_ready_to_transmit(void);

#endif /* STM32G4xx_ADUIO_H */
