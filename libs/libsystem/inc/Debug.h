#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdint.h>

typedef enum {
	DEBUG_NONE,
	DEBUG_USART,
	DEBUG_ITM
//	DEBUG_USB
} DebugDevice;

extern DebugDevice CurrentDebugDevice;

void EnableDebugOutput(DebugDevice device);
void DebugPrintChar(char c);

#endif
