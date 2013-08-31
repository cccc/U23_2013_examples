#include "main.h"

//el cheapo delay function
void Delay(__IO uint32_t nCount)
{
	while(nCount--);
}

int main()
{
	//Enable clock for GPIOD peripheral
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	//Initialize GPIOs for LEDs
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
		Delay(10000000L);

		//set PD13 (orange LED)
		GPIOD->BSRRL = GPIO_Pin_13;
		//reset other pins
		GPIOD->BSRRH = GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15;
		Delay(10000000L);

		//set PD14 (red LED)
		GPIOD->BSRRL = GPIO_Pin_14;
		//reset other pins
		GPIOD->BSRRH = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
		Delay(10000000L);

		//set PD15 (blue LED)
		GPIOD->BSRRL = GPIO_Pin_15;
		//reset other pins
		GPIOD->BSRRH = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;
		Delay(10000000L);
	}
}
