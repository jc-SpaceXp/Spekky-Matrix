#ifndef FFT_RTOS_H
#define FFT_RTOS_H

#include "FreeRTOS.h"
#include "timers.h"

void fft_oneshot_callback(xTimerHandle pxTimer);

#endif /* FFT_RTOS_H */
