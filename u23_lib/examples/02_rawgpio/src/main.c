#include <System.h>
#include <stm32f4xx.h>

int main()
{
	// Do some basic initialization tasks
	InitializeSystem();

	// Enable clock for GPIOD peripheral
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// Set Pin modes for PD12 to PD15 (LEDs)
	GPIO_Init(GPIOD, &(GPIO_InitTypeDef){
		.GPIO_Speed = GPIO_Speed_50MHz,
		.GPIO_Mode = GPIO_Mode_OUT,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_UP,
		.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15
	});

	while (1)
	{
		//BSRRL: Bit Set/Reset Register Low
		//BSRRH: Bit Set/Reset Register High

		//Setting GPIO with direct register access
		//set PD12 (green LED)
		GPIOD->BSRRL = GPIO_Pin_12;
		//reset other pins
		GPIOD->BSRRH = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
		Delay(100);

		//Settings GPIO using peripheral lib helper functions
		GPIO_SetBits(GPIOD, GPIO_Pin_13);
		GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15);
		Delay(100);

		//Bit toggling using peripheral lib
		GPIO_ToggleBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
		Delay(100);

		//Manual switching with bit operations
		uint16_t gpioData = GPIO_ReadOutputData(GPIOD);							//get current GPIO state
		gpioData &= ~(GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);	//set all bits to zero using a mask
		gpioData |= (GPIO_Pin_13 | GPIO_Pin_14);								//set bit (in this case pin) 13 and 14 again
		GPIO_Write(GPIOD, gpioData);											//set modified data again
		Delay(100);
	}
}
