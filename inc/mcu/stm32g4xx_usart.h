#ifndef STM32G4xx_USART_H
#define STM32G4xx_USART_H

#include <stdbool.h>

// Don't use GPIO_PIN_x as this is the bit mask not pin number
#define USART_TX_PIN    9
#define USART_TX_PORT   GPIOA

void setup_hw_usart(void);

#endif /* STM32G4xx_USART_H */
