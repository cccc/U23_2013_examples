#include <System.h>
#include <stm32f4xx.h>

int main()
{
	// Do some basic initialization tasks
	InitializeSystem();

	// Enable clock for the system configuration controller
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

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

	//We are going to use the External interrupt/event controller (EXTI) here
	//It "watches" a pin for an event and triggers an interrupt
	//See STM32 Reference Manual Page 199ff. for detailed functional description

	//PA0 (the user-button), PB0, PC0, etc. are connected to EXTI0 (page 202)
	//So we configure that...
	EXTI_Init(&(EXTI_InitTypeDef){
		.EXTI_Line = EXTI_Line0,
		.EXTI_Mode = EXTI_Mode_Interrupt,
		.EXTI_Trigger = EXTI_Trigger_Rising,
		.EXTI_LineCmd = ENABLE
	});

	//Now we configured the EXTI-Controller and the GPIO-Controller
	//Wait... If PA0, PB0, PC0, PD0, etc. are all connected to EXTI0, do the all trigger an interrupt?

	//No, we need to select which GPIO line is actively connected to the EXTI
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

	//Now the EXTI tells the NVIC (Nested Vectored Interrupt Controller) if something happened
	//The interrupt controller is going to ignore that event, we didn't configure it yet
	//Our standard configuration of the NVIC is 4 bit for the preemption priority and 0 bit for the subpriority
	//see the header in libs/libstm32f4xx/src/misc.c for more information
	NVIC_Init(&(NVIC_InitTypeDef){
		.NVIC_IRQChannel = EXTI0_IRQn,
		.NVIC_IRQChannelPreemptionPriority = 0x00,	//lowest priority possible
		.NVIC_IRQChannelSubPriority = 0x00,
		.NVIC_IRQChannelCmd = ENABLE
	});

	//now we should finally be able to receive interrupts \o/

	while (1)
	{
		//Do other important stuff...
	}
}

//The interrupt handler called automatically
void EXTI0_IRQHandler()
{
	//huh? You talking to me?
	if(EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		GPIO_ToggleBits(GPIOD, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);

		//Clear the interrupt bit and tell the controller we handlet the interrupt
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}
