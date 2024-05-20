#ifndef LED_MATRIX_RTOS_H
#define LED_MATRIX_RTOS_H

#include "FreeRTOS.h"
#include "timers.h"

void led_matrix_setup(void);
void led_matrix_update_callback(xTimerHandle pxTimer);

#endif /* LED_MATRIX_RTOS_H */
