#ifndef LED_MATRIX_PRIVATE_H
#define LED_MATRIX_PRIVATE_H

#include <stdint.h>

struct LedSpiPin {
	volatile uint32_t* assert_address;
	volatile uint32_t* deassert_address;
	unsigned int pin;
};

struct MaximMax2719 {
	struct LedSpiPin cs; // CS, chip select
};

#endif /* LED_MATRIX_PRIVATE_H */
