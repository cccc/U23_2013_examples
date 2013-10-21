#include <System.h>
#include <stm32f4xx.h>

int main()
{
	// Do some basic initialization tasks
	InitializeSystem();

	// Enable clock for GPIOE peripheral
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	// Set Pin modes for PE5
	GPIO_Init(GPIOE, &(GPIO_InitTypeDef){
		.GPIO_Speed = GPIO_Speed_100MHz,
		.GPIO_Mode = GPIO_Mode_AF,		//Not OUT, but AF = Alternate Function
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_NOPULL,
		.GPIO_Pin = GPIO_Pin_5
	});

	//Attach pin 5 of GPIOE to alternate pin function on TIM9
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource5, GPIO_AF_TIM9);

	//We are using TIM9 here

	//The timer is connected to APB2 which has a prescaler of 2
	//The clock for TIM1,8,9,10,11 is then agin multiplied by 2
	//Our main system clock is 168MHz, so the timer ticks with 168MHz/2*2 = 168MHz

	//More about the clocking: http://myembeddedtutorial.blogspot.de/2013/06/working-with-stm32f4-timers.html
	//The clocks are configured in libs/libCMSIS/DeviceSupport/ST/STM32F4xx/system_stm32f4xx.c
	//There is a clocking wizard Excel sheet in /docs (you need Excel! No OpenOffice!) which generates the above file

	//Enable Timer 9 clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);

	//We want the timer to tick with a frequency of 2MHz, calculate a prescaler
	uint32_t PrescalerValue = (uint16_t) (SystemCoreClock / 1000000) - 1;

	//One period should last 20ms
	//We can then use a compare value of 2000 (=1ms high time) for the lowest servo position and 4000 (=2ms high time) for the highest position
	uint32_t period = 20000;

	//Configure the timer
	TIM_TimeBaseInit(TIM9, &(TIM_TimeBaseInitTypeDef){
		.TIM_Period = period - 1,
		.TIM_Prescaler = PrescalerValue,
		.TIM_ClockDivision = 0,
		.TIM_CounterMode = TIM_CounterMode_Up,
	});

	// PWM1 Mode configuration: Channel1 (GPIOE Pin 5)
	TIM_OC1Init(TIM9, &(TIM_OCInitTypeDef){
		.TIM_OCMode = TIM_OCMode_PWM1,
		.TIM_OutputState = TIM_OutputState_Enable,
		.TIM_Pulse = 0,
		.TIM_OCPolarity = TIM_OCPolarity_High,
	});
	TIM_OC1PreloadConfig(TIM9, TIM_OCPreload_Enable);

	//Enable the timer
	TIM_Cmd(TIM9, ENABLE);


	uint32_t position = 1500;

	while (1)
	{
		TIM_SetCompare1(TIM9, position);
		Delay(100);

		position += 100;

		if(position > 2000)
		 	position = 1000;
	}
}
