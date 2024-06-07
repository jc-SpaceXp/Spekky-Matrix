#ifndef STM32G4xx_I2S_H
#define STM32G4xx_I2S_H

#include <stdbool.h>

// Don't use GPIO_PIN_x as this is the bit mask not pin number
// Mainly using pin out of SPI2S (which is AF5)
#define I2S_CLK_PIN    1
#define I2S_CLK_PORT   GPIOF  // (SPI2 CLK)
#define I2S_SD_PIN     11
#define I2S_SD_PORT    GPIOA  // (SPI2 MOSI)
#define I2S_WS_PIN     0      // Treated as GPIO, see no 12
#define I2S_WS_PORT    GPIOF

void setup_hw_i2s(void);
bool i2s_rx_data_in_buffer(void);
bool i2s_rx_is_lchannel(void);
bool i2s_rx_is_rchannel(void);

#endif /* STM32G4xx_I2S_H */
