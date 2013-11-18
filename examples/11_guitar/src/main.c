#include <stdio.h>
#include <System.h>

#include "guitar.h"

#define AudioFreq AudioFreq_44k

static const int SPEED = 400;

static void AudioCallback(void *context, int16_t buffer[256])
{
	static uint32_t pos = 0;

	for(int i = 0; i < 128; i++) {
		buffer[2*i+0] = buffer[2*i+1] = guitar[pos];
		pos = (pos + 1) % 220500; // 5 sec 44.1 kHz Audio
	}
}

int main(void)
{
	uint16_t led = 0;

	// Do some basic initialization tasks
	InitializeSystem();

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_Init(GPIOA, &(GPIO_InitTypeDef){
		.GPIO_Speed = GPIO_Speed_50MHz,
		.GPIO_Mode = GPIO_Mode_IN,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_DOWN,
		.GPIO_Pin = GPIO_Pin_0
	});

	InitializeAudio(AudioFreq);

	// Enable printf via trace macrocell (get output with 'make trace')
	EnableDebugOutput(DEBUG_ITM);

	InitializeLEDs();
	SetLEDs(1 | 2 | 4 | 8);

	iprintf("Audio!\r\n");

	SetAudioVolume(0xa0);
	PlayAudioWithCallback(AudioCallback, NULL);

	int cnt = 0;
	while (1) {
		if ((led++ % SPEED) == 0) {
			if (led >= SPEED * 4) led = 0;
			SetLEDs(1<<(led / SPEED));
		}
		Delay(1);
	}

	return 0;
}
