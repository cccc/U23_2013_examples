#include <System.h>

#include <stdio.h>

int main()
{
	// Do some basic initialization tasks
	InitializeSystem();

	// Initialize pins for LEDs
	InitializeLEDs();

	EnableDebugOutput(DEBUG_ITM);

	while (1)
	{
		SetLEDs(1);
		Delay(100);

		SetLEDs(2);
		Delay(100);

		SetLEDs(4);
		Delay(100);

		SetLEDs(8);
		Delay(100);

		iprintf("Done\r\n");
	}
}
