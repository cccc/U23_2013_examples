#include <System.h>
#include <stdio.h>
#include <stm32f4xx.h>

#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
 
int ConvertedValue = 0; //Converted value readed from ADC
 
 //lustiges Beispiel aus dem Internet (für Feuchtigkeitssensor)
//von http://myactivities-mazen.blogspot.de/2013/04/adc-with-stm32f4.html
void adc_configure(){
 ADC_InitTypeDef ADC_init_structure; //Structure for adc confguration
 GPIO_InitTypeDef GPIO_initStructre; //Structure for analog input pin
 //Clock configuration
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);//The ADC1 is connected the APB2 peripheral bus thus we will use its clock source
 RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_GPIOCEN,ENABLE);//Clock for the ADC port!! Do not forget about this one ;)
 //Analog pin configuration
 GPIO_initStructre.GPIO_Pin = GPIO_Pin_0;//The channel 10 is connected to PC0
 GPIO_initStructre.GPIO_Mode = GPIO_Mode_AN; //The PC0 pin is configured in analog mode
 GPIO_initStructre.GPIO_PuPd = GPIO_PuPd_NOPULL; //We don't need any pull up or pull down
 GPIO_Init(GPIOC,&GPIO_initStructre);//Affecting the port with the initialization structure configuration
 //ADC structure configuration
 ADC_DeInit();
 ADC_init_structure.ADC_DataAlign = ADC_DataAlign_Right;//data converted will be shifted to right
 ADC_init_structure.ADC_Resolution = ADC_Resolution_12b;//Input voltage is converted into a 12bit number giving a maximum value of 4096
 ADC_init_structure.ADC_ContinuousConvMode = ENABLE; //the conversion is continuous, the input data is converted more than once
 ADC_init_structure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;// conversion is synchronous with TIM1 and CC1 (actually I'm not sure about this one :/)
 ADC_init_structure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//no trigger for conversion
 ADC_init_structure.ADC_NbrOfConversion = 1;//I think this one is clear :p
 ADC_init_structure.ADC_ScanConvMode = DISABLE;//The scan is configured in one channel
 ADC_Init(ADC1,&ADC_init_structure);//Initialize ADC with the previous configuration
 //Enable ADC conversion
 ADC_Cmd(ADC1,ENABLE);
 //Select the channel to be read from
 ADC_RegularChannelConfig(ADC1,ADC_Channel_10,1,ADC_SampleTime_144Cycles);
}
int adc_convert(){
 ADC_SoftwareStartConv(ADC1);//Start the conversion
 while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));//Processing the conversion
 return ADC_GetConversionValue(ADC1); //Return the converted data
}


int main()
{
	// Do some basic inGPIO_Pin_0itialization tasks
	InitializeSystem();
	EnableDebugOutput(DEBUG_ITM);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_Init(GPIOA, &(GPIO_InitTypeDef){
		.GPIO_Speed = GPIO_Speed_50MHz,
		.GPIO_Mode = GPIO_Mode_IN,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_NOPULL,
		.GPIO_Pin = GPIO_Pin_0
	});

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_Init(GPIOD, &(GPIO_InitTypeDef){
		.GPIO_Speed = GPIO_Speed_50MHz,
		.GPIO_Mode = GPIO_Mode_OUT,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_UP,
		.GPIO_Pin = GPIO_Pin_12
	});


	GPIO_SetBits(GPIOD, GPIO_Pin_12);


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_Init(GPIOB, &(GPIO_InitTypeDef){
		.GPIO_Speed = GPIO_Speed_50MHz,
		.GPIO_Mode = GPIO_Mode_IN,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_NOPULL,
		.GPIO_Pin = GPIO_Pin_12
	});
	iprintf("Vor adc_configure\n");
	adc_configure();

	while(1)
	{
			
			iprintf("Pin 12:%i ",GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12));
			iprintf("Pin 13:%i ",GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13));

     	iprintf("Schwarze Magie:%i\n",adc_convert());//Read the ADC converted value
    	Delay(10);
	}
}




