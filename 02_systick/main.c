#include "main.h"

//A tick counter
static volatile uint32_t SysTickCounter=0;

//Interrupt handler for the SysTick counter
void SysTick_Handler()
{
	SysTickCounter++;
}

// Proper delay function based on the SysTick counter
void Delay(uint32_t milliseconds)
{
	volatile uint32_t end=SysTickCounter+milliseconds;
	while(SysTickCounter!=end);
}


int main()
{
	// Get information about the configured clock frequencies
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);

	// Set the tick rate of the SysTick timer to 1ms
	// RCC_Clocks.HCLK_Frequency contains the amount of ticks of the counter
	// per second divided by 1000 we get the amount of ticks per millisecond
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

	// Configure the interrupt controller to have 16 different priorities
	// and no subpriorities
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	// Enable clock for GPIOD peripheral
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// Initialize GPIOs for LEDs
	GPIO_Init(GPIOD, &(GPIO_InitTypeDef){
		.GPIO_Speed = GPIO_Speed_50MHz,
		.GPIO_Mode = GPIO_Mode_OUT,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_UP,
		.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13
		            | GPIO_Pin_14 | GPIO_Pin_15
	});

	while (1)
	{
		//set PD12 (green LED)
		GPIOD->BSRRL = GPIO_Pin_12;
		//reset other pins
		GPIOD->BSRRH = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
		Delay(500);

		//set PD13 (orange LED)
		GPIOD->BSRRL = GPIO_Pin_13;
		//reset other pins
		GPIOD->BSRRH = GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15;
		Delay(500);

		//set PD14 (red LED)
		GPIOD->BSRRL = GPIO_Pin_14;
		//reset other pins
		GPIOD->BSRRH = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
		Delay(500);

		//set PD15 (blue LED)
		GPIOD->BSRRL = GPIO_Pin_15;
		//reset other pins
		GPIOD->BSRRH = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
		Delay(500);
	}
}
