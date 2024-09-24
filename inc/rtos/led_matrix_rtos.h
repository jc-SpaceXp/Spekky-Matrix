#ifndef LED_MATRIX_RTOS_H
#define LED_MATRIX_RTOS_H

#include "FreeRTOS.h"

void led_matrix_setup(int total_devices);
void led_matrix_update_callback(void* pvParameters);

#endif /* LED_MATRIX_RTOS_H */
