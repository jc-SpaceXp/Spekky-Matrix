#include "led_matrix.h"

void set_spi_pin_details(struct LedSpiPin* spi_pin
                        , volatile uint32_t* assert_addr
                        , volatile uint32_t* deassert_addr
                        , unsigned int pin)
{
	*spi_pin = ((struct LedSpiPin) { assert_addr, deassert_addr, pin } );
}

void set_led_cs_pin_details(struct LedSpiPin* dest, const struct LedSpiPin* src)
{
	*dest = *src;
}
