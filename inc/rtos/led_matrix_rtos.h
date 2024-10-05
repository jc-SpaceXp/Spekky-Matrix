#ifndef LED_MATRIX_RTOS_H
#define LED_MATRIX_RTOS_H

void led_matrix_setup(int total_devices);
void led_matrix_update_task(void* pvParameters);

#endif /* LED_MATRIX_RTOS_H */
