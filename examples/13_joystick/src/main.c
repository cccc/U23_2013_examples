#include <System.h>
#include <stdio.h>

/* This example is loosely based on 12_adc and can be used to read the raw data of a joystick.
 * (s. http://dx.com/p/meeeno-ps2-joystick-module-stick-for-arduino-orange-black-223662)
 * Connect the "X" pin to PC0, the "Y" pin to PC1 and the "K" pin to PA1 in order to
 * get some output of the form "<voltage @ X>, <voltage @ Y> | <button pressed?>".
 */
int main()
{
	/* basic initializations */
	InitializeSystem();
	InitializeLEDs();
	EnableDebugOutput(DEBUG_ITM);

	/* enable the clocks required */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); /* pin for the button */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); /* pins for ADC */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); /* ADC1 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE); /* ADC2 */

	/* initialize the pins in the respective modes */
	GPIO_Init(GPIOA, &(GPIO_InitTypeDef){
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode = GPIO_Mode_IN,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd = GPIO_PuPd_UP, /* The button needs pullup. */
                .GPIO_Pin = GPIO_Pin_1
        });
        GPIO_Init(GPIOC, &(GPIO_InitTypeDef){
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode = GPIO_Mode_AN,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd = GPIO_PuPd_NOPULL,
                .GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1
        });

	/* initialize ADCs:
	 * 12b resolution gives us results between 0 and 4095.
	 * The rest is just a configuration that works,
	 * it would probably still work after some small changes.
	 */
	ADC_Init(ADC1, &(ADC_InitTypeDef){
		.ADC_Resolution = ADC_Resolution_12b,
		.ADC_ScanConvMode = DISABLE,
		.ADC_ContinuousConvMode = ENABLE, 
		.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None,
		.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1,
		.ADC_DataAlign = ADC_DataAlign_Right,
		.ADC_NbrOfConversion = 1
        });
	ADC_Init(ADC2, &(ADC_InitTypeDef){
		.ADC_Resolution = ADC_Resolution_12b, 
		.ADC_ScanConvMode = DISABLE,
		.ADC_ContinuousConvMode = ENABLE, 
		.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None,
		.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1,
		.ADC_DataAlign = ADC_DataAlign_Right,
		.ADC_NbrOfConversion = 1
        });

	/* set the channels and start the ADCs
	 * TODO: use only one ADC for both inputs 
 	 */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_3Cycles);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_11, 1, ADC_SampleTime_3Cycles);
	ADC_Cmd(ADC1, ENABLE);
	ADC_Cmd(ADC2, ENABLE);

	while(1){
		uint16_t values[3];

		/* read data */
		ADC_SoftwareStartConv(ADC1);
		while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
		values[0] = ADC_GetConversionValue(ADC1);

		ADC_SoftwareStartConv(ADC2);
		while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC));
		values[1] = ADC_GetConversionValue(ADC2);

		values[2] = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1);
 
		/* scale the voltages to 5V and remember that the button bit is normally SET
		 * FIXME: Even though the DX page says that the voltages at "X" and "Y" should be
		 * around 2.5V when the joystick is not moved, the shown voltages are around 3.8-4.0V
		 * when scaled as below; i.e. something is wrong with DX, the pins or our scaling.
		 */
		printf("%f, %f | %i\n", 5*(values[0]/4095.0), 5*(values[1]/4095.0),  values[2] == Bit_RESET);

		Delay(5);
	}
}
