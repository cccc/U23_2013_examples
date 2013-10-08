#include <Debug.h>
#include <System.h>

#include <stdio.h>

DebugDevice CurrentDebugDevice = DEBUG_NONE;

void EnableDebugOutput(DebugDevice device)
{
	if(device == DEBUG_USART)
	{
		MyUSART_Init();
		setvbuf(stdout, 0, _IONBF, 0);
	}

	CurrentDebugDevice = device;
}

void DebugPrintChar(char c)
{
	if(CurrentDebugDevice == DEBUG_USART)
		USART_SendChar(c);
	else if(CurrentDebugDevice == DEBUG_ITM)
		ITM_SendChar(c);
}
