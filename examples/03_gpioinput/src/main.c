#include <System.h>
#include <stm32f4xx.h>

int main()
{
	// Do some basic initialization tasks
	InitializeSystem();

	// Enable clock for GPIOD peripheral
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// Enable clock for GPIOA peripheral
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// Set Pin modes for PD12 to PD15 (LEDs)
	GPIO_Init(GPIOD, &(GPIO_InitTypeDef){
		.GPIO_Speed = GPIO_Speed_50MHz,
		.GPIO_Mode = GPIO_Mode_OUT,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_UP,
		.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15
	});

	// Set Pin mode for PA0 (User-Button)
	GPIO_Init(GPIOA, &(GPIO_InitTypeDef){
		.GPIO_Speed = GPIO_Speed_50MHz,
		.GPIO_Mode = GPIO_Mode_IN,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_NOPULL,	//no internal pullup or pulldown, is present on PCB
		.GPIO_Pin = GPIO_Pin_0
	});


	while (1)
	{
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_SET)
		{
			GPIO_SetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_15);
			GPIO_ResetBits(GPIOD, GPIO_Pin_13 | GPIO_Pin_14);
		}
		else
		{
			GPIO_ResetBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_15);
			GPIO_SetBits(GPIOD, GPIO_Pin_13 | GPIO_Pin_14);
		}
	}
}
