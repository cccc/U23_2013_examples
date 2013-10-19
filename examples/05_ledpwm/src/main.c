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
		.GPIO_Speed = GPIO_Speed_100MHz,
		.GPIO_Mode = GPIO_Mode_AF,		//Not OUT, but AF = Alternate Function
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_UP,
		.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15
	});

	//Attach pin 12 to 15 of GPIOD to alternate pin function on TIM4
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_TIM4);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_TIM4);


	//We are going to use a timer to change the brightness of the LEDs

	//We are using TIM4 here
	//It is a 16 Bit timer
	//We are going to use 4 channels of this timer (each LED gets its own channel)

	//The timer is connected to APB1 which has a prescaler of 4
	//The clock for TIM2,3,4,5,6,7,12,13,14 is then agin multiplied by 2
	//Our main system clock is 168MHz, so the timer ticks with 168MHz/4*2 = 168MHz/2 = 84MHz

	//More about the clocking: http://myembeddedtutorial.blogspot.de/2013/06/working-with-stm32f4-timers.html
	//The clocks are configured in libs/libCMSIS/DeviceSupport/ST/STM32F4xx/system_stm32f4xx.c
	//There is a clocking wizard Excel sheet in /docs (you need Excel! No OpenOffice!) which generates the above file

	//Enable Timer 4 clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	//We want the timer to tick with a frequency of 1MHz, calculate a prescaler
	uint32_t PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 1000000) - 1;

	//20 kHz PWM period (i.e. 50uS period)
	uint32_t period = 1000000 / 20000;

	//Configure the timer
	TIM_TimeBaseInit(TIM4, &(TIM_TimeBaseInitTypeDef){
		.TIM_Period = period - 1,
		.TIM_Prescaler = PrescalerValue,
		.TIM_ClockDivision = 0,
		.TIM_CounterMode = TIM_CounterMode_Up,
	});

	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	// PWM1 Mode configuration: Channel1 (GPIOD Pin 12)
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

	// PWM1 Mode configuration: Channel2 (GPIOD Pin 13)
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

	// PWM1 Mode configuration: Channel3 (GPIOD Pin 14)
	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);

	// PWM1 Mode configuration: Channel4 (GPIOD Pin 15)
	TIM_OC4Init(TIM4, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);

	//Enable the timer
	TIM_Cmd(TIM4, ENABLE);

	//low duty cycles are not visible, so we set a maximum brigthness here
	uint32_t max_brightness = period / 4;

	//Set some start values
	uint32_t brightLed1 = 1;						//0% brightness
	uint32_t brightLed2 = max_brightness / 3;		//33% brightness
	uint32_t brightLed3 = (max_brightness / 3) * 2;	//66% brightness
	uint32_t brightLed4 = max_brightness - 1;		//100% brightness

	int direction1 = 1;
	int direction2 = 1;
	int direction3 = 1;
	int direction4 = 1;

	//Set some initial compare values
	TIM_SetCompare1(TIM4, brightLed1);
	TIM_SetCompare2(TIM4, brightLed2);
	TIM_SetCompare3(TIM4, brightLed3);
	TIM_SetCompare4(TIM4, brightLed4);

	while (1)
	{
		Delay(4);

		//Make the counter count up and down alternately
		if(direction1 == 1)
			brightLed1++;
		else
			brightLed1--;

		//change the direction if we hit the upper or lower bound
		if(brightLed1 == max_brightness || brightLed1 == 0)
			direction1 *= -1;


		if(direction2 == 1)
			brightLed2++;
		else
			brightLed2--;

		if(brightLed2 == max_brightness || brightLed2 == 0)
			direction2 *= -1;


		if(direction3 == 1)
			brightLed3++;
		else
			brightLed3--;

		if(brightLed3 == max_brightness || brightLed3 == 0)
			direction3 *= -1;


		if(direction4 == 1)
			brightLed4++;
		else
			brightLed4--;

		if(brightLed4 == max_brightness || brightLed4 == 0)
			direction4 *= -1;

		//Update compare values
		TIM_SetCompare1(TIM4, brightLed1);
		TIM_SetCompare2(TIM4, brightLed2);
		TIM_SetCompare3(TIM4, brightLed3);
		TIM_SetCompare4(TIM4, brightLed4);
	}
}
