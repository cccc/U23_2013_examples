#include "main.h"

// FIXME: more explanation

// NOTE: Timers can be very complex! The reference manual is your friend.

static volatile uint32_t SysTickCounter=0;

void SysTick_Handler()
{
	SysTickCounter++;
}

void Delay(uint32_t milliseconds)
{
	volatile uint32_t end=SysTickCounter+milliseconds;
	while(SysTickCounter!=end);
}

int main()
{
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);

	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);


	// Enable clock for GPIOD peripheral
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// Initialize GPIOs for LEDs
	// Set the mode to AF = Auxiliary Function as they are going to be
	// controlled by a timer
	GPIO_Init(GPIOD, &(GPIO_InitTypeDef){
		.GPIO_Speed = GPIO_Speed_100MHz,
		.GPIO_Mode = GPIO_Mode_AF,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_UP,
		.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13
		            | GPIO_Pin_14 | GPIO_Pin_15
	});

	// Enable clock for Timer 4
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	// Tell the pin multiplexing matrix that it should connect the GPIO pins
	// actually to the auxiliary function pins
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_TIM4);

	// SystemCoreClock is 168MHz
	// (see CMSIS/DeviceSupport/ST/STM32F4xx/system_stm32f4xx.c)
	// APB1 (the bus system TIM4 is attached to) prescaler is 4
	// that makes the clock frequency of TIM4 168MHz/4 = 42MHz
	// Divided by 21*10^6 sets the prescaler to 2 and makes
	// the timer ticks with 21MHz, e.g. its value is increated 21 million
	// times per second  the -1 at the end is only for ofsetting the
	// calculated value: 0 = prescaler of 1, 1 = prescaler of 2 etc.
	uint16_t PrescalerValue = (uint16_t) ((SystemCoreClock / 4) / 21000000) - 1;

	// Note: TIM_Period is the Autoreload value.
	// As soon as the counter overflows, this value is loaded and the counter
	// is started again
	TIM_TimeBaseInit(TIM4, &(TIM_TimeBaseInitTypeDef){
		.TIM_Period = 665,
		.TIM_Prescaler = PrescalerValue,
		.TIM_ClockDivision = 0,
		.TIM_CounterMode = TIM_CounterMode_Up
	});

	// Configure Output compare channels 1 to 4 for timer 4
	TIM_OCInitTypeDef TIM_OCInitStructure = {
		.TIM_OCMode = TIM_OCMode_PWM1,
		.TIM_OutputState = TIM_OutputState_Enable,
		.TIM_Pulse = 0,
		.TIM_OCPolarity = TIM_OCPolarity_High
	};

	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

	TIM_OC2Init(TIM4, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);

	TIM_OC4Init(TIM4, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);

	// Enable buffering of the ARR register
	// If the auto reload value should be changed, the change takes effect
	// after the next update of the timer, not immediately
	TIM_ARRPreloadConfig(TIM4, ENABLE);

	// Enable the timer
	TIM_Cmd(TIM4, ENABLE);

	int brightness = 0;
	while (1)
	{
		brightness++;

		// Set channels' compare values and change the brightness
		TIM4->CCR3 = 333 - (brightness + 0) % 333;
		TIM4->CCR4 = 333 - (brightness + 166/2) % 333;
		TIM4->CCR1 = 333 - (brightness + 333/2) % 333;
		TIM4->CCR2 = 333 - (brightness + 499/2) % 333;

		Delay(5);
	}
}
