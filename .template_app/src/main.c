#include <System.h>

#include <stdio.h>

int main()
{
	// Do some basic initialization tasks
	InitializeSystem();

	// Initialize pins for LEDs
	InitializeLEDs();

	// Enable printf via trace macrocell (get output with 'make trace')
	EnableDebugOutput(DEBUG_ITM);

	//Turn on all LEDs
	SetLEDs(1 | 2 | 4 | 8);

	iprintf("Hello, World!\r\n");

	while(1);
}
