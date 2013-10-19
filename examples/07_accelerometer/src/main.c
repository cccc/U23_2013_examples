#include <System.h>
#include <Accelerometer.h>

int main()
{
	// Do some basic initialization tasks
	InitializeSystem();

	// Initialize pins for LEDs
	InitializeLEDs();

	// Initialize pins for Accelerometer
	InitializeAccelerometer();

	// Calibrate the Accelerometer on the Start
	CalibrateAccelerometer();

	while (1)
	{
		int led = 0;
		int8_t Data[3] = {};

		// Read new Data
		ReadCalibratedAccelerometerData(Data);

		// Look at the roll
		if(Data[0] > 8)
		{
			led += 8; // LED near User-Button
		}
		else if(Data[0] < -8)
		{
			led += 2; // LED near Reset-Button
		}

		// Look at the Pitch
		if(Data[1] > 8)
		{
			led += 4; // LED near Mic
		}
		else if(Data[1] < -8)
		{
			led += 1; // LED near CPU
		}

		// Set the LEDs
		SetLEDs(led);
	}
}
