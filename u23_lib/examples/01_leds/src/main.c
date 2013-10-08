#include <System.h>

int main()
{
	// Do some basic initialization tasks
	InitializeSystem();

	// Initialize pins for LEDs
	InitializeLEDs();

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
	}
}
