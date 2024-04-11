/*
 * FreeRTOS V202212.01
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION               1
#define configUSE_IDLE_HOOK                0
#define configUSE_TICK_HOOK                0
#define configCPU_CLOCK_HZ                 ( ( unsigned long ) 8000000 ) // HCLK (w/ prescaler of 1/2)
#define configSYSTICK_CLOCK_HZ             ( configCPU_CLOCK_HZ / 8 ) // CortexM specific, only used if SysTick is clocked differently to CPU clock
#define configTICK_RATE_HZ                 ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES	           ( 5 )
#define configMINIMAL_STACK_SIZE           ( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE              ( ( size_t ) ( 12 * 1024 ) )
#define configMAX_TASK_NAME_LEN            ( 11 ) // includes NULL char
#define configUSE_TRACE_FACILITY           0
#define configUSE_16_BIT_TICKS             0
#define configIDLE_SHOULD_YIELD            1
#define configUSE_MUTEXES                  0
#define configCHECK_FOR_STACK_OVERFLOW     2

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES              0
#define configMAX_CO_ROUTINE_PRIORITIES    ( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet           0
#define INCLUDE_uxTaskPriorityGet          0
#define INCLUDE_vTaskDelete                0
#define INCLUDE_vTaskCleanUpResources      0
#define INCLUDE_vTaskSuspend               0
#define INCLUDE_vTaskDelayUntil            0
#define INCLUDE_vTaskDelay                 1

// Cortex M specific defintions
#ifdef __NVIC_PRIO_BITS // CMSIS defines this
	#define configPRIO_BITS                __NVIC_PRIO_BITS
#else
	#define configPRIO_BITS                4
#endif /* __NVIC_PRIO_BITS */

// STMG4 has 16 programmable priority levels, 4 bits of interrupt priority
// Again highest values correspons to the lowest interrupt priority e.g. 15 == 255
// These are prefixed w/ configLIBRARY, other config interrupts will be mapped to 0-255
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

// Cortex M3-M4 warning:
// Interrupt priorities must not be set to 0
// As 0 (and negative numbers) are the highest priority interrupts
// A low number will guarantee that an interrupt will occur
// http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) ) // 1111 < 4 --> 1111 0000 (0xF0, 240)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) ) // 0101 < 4 --> 0101 0000 (0x50, 80)

// Catch any asserts failures w/o the need for the assert.h header
// Does increase code size, disable after the app is stable and no longer testing
#define configASSERT( x ) if ( ( x ) == 0 )     { taskDISABLE_INTERRUPTS(); for( ;; ); }

// Map FreeRTOS interrupts to their CMSIS counterparts
#define vPortSVCHandler         SVC_Handler
#define xPortPendSVHandler      PendSV_Handler
#define xPortSysTickHandler     SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
