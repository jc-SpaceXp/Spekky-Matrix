#ifndef STM32G4xx_GPIO_DEBUG_H
#define STM32G4xx_GPIO_DEBUG_H

#include <stdbool.h>

#define GPIO_DEBUG_PIN   4 // unused in led matrix
#define GPIO_DEBUG_PORT   GPIOB

void setup_hw_gpio_debug(void);

#endif /* STM32G4xx_GPIO_DEBUG_H */
