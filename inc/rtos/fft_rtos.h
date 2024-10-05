#ifndef FFT_RTOS_H
#define FFT_RTOS_H

#include "FreeRTOS.h"
#include "timers.h"

void fft_task_processing(void* pvParameters);

#endif /* FFT_RTOS_H */
