#ifndef STM32G4xx_SPI_H
#define STM32G4xx_SPI_H

#include <stdbool.h>

// Don't use GPIO_PIN_x as this is the bit mask not pin number
#define SPI_CLK_PIN    3
#define SPI_CLK_PORT   GPIOB
#define SPI_MISO_PIN   4 // unused in Adafruit ST7789 display
#define SPI_MISO_PORT  GPIOB
#define SPI_MOSI_PIN   5
#define SPI_MOSI_PORT  GPIOB
#define SPI_CS_PIN     11     // Treated as GPIO
#define SPI_CS_PORT    GPIOA
// GPIO managed pins, additional signals for ST7789
#define GPIO_DCX_PIN     12
#define GPIO_DCX_PORT    GPIOA
#define GPIO_RSX_PIN     0
#define GPIO_RSX_PORT    GPIOA

void setup_hw_spi(void);
bool tx_ready_to_transmit(void);
bool tx_complete(void);

#endif /* STM32G4xx_SPI_H */
