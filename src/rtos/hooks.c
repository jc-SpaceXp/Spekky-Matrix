#include "FreeRTOS.h"
#include "task.h"

// Must define when using a non-zero config for stack overflow
void vApplicationStackOverflowHook(xTaskHandle pxTask, char *pcTaskName)
{
	// remove unused warnings from compiler
	(void) pxTask;
	(void) pcTaskName;
	for (;;) {
		// do nothing
	}
}
