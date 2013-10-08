#include <Init.h>
#include <System.h>

void InitializeSystem(void)
{
	// Configure the interrupt controller to have 16 different priorities
	// and no subpriorities
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	// Get information about the configured clock frequencies
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);

	// Set the tick rate of the SysTick timer to 10ms
	// RCC_Clocks.HCLK_Frequency contains the amount of ticks of the counter
	// per second divided by 1000 we get the amount of ticks per millisecond
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);
}
