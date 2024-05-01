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

void setup_hw_spi(void);
bool spi_tx_ready_to_transmit(void);
bool spi_tx_complete(void);

#endif /* STM32G4xx_SPI_H */
