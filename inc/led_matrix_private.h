#ifndef LED_MATRIX_PRIVATE_H
#define LED_MATRIX_PRIVATE_H

#include <stdint.h>

struct LedSpiPin {
	volatile uint32_t* assert_address;
	volatile uint32_t* deassert_address;
	unsigned int pin;
};

struct MaximMax7219 {
	struct LedSpiPin cs; // CS, chip select
	int total_devices;
};

#endif /* LED_MATRIX_PRIVATE_H */
