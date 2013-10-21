#include <System.h>
#include <Synth.h>

#include <stdio.h>

#define AudioFreq Gaming_AudioFreq_22k

static const int SPEED = 40;

SynthSong song = {
	AudioFreq,
	1,
	&(SynthChannel) {
		SynthRect,
		19*2,
		(SynthNote[]) {
			{ Note_a1, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_a1, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_c2, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_a1, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_a2, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_e2, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_d2, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_c2, 180, 0xff },
			{ Note_Pause, 40, 0x00 },
			{ Note_d2, 180, 0xff },
			{ Note_Pause, 40, 0x00 },
			{ Note_e2, 180, 0xff },
			{ Note_Pause, 40, 0x00 },
			{ Note_a1, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_a1, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_c2, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_a1, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_h1, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_h1, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_a1, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_c2, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
			{ Note_g1, 250, 0xff },
			{ Note_Pause, 62, 0x00 },
		}
	}
};

static void AudioCallback(void *context, int16_t buffer[256])
{
	iprintf("Called AudioCallback()\n");
	for(int i = 0; i < 128; i++) {
		buffer[2*i+0] = buffer[2*i+1] = SynthGetSample(&song);
	}
}

int main(void)
{
	uint16_t led = 0;

	// Do some basic initialization tasks
	InitializeSystem();

	InitializeAudio(AudioFreq);

	// Enable printf via trace macrocell (get output with 'make trace')
	EnableDebugOutput(DEBUG_ITM);

	InitializeLEDs();
	SetLEDs(1 | 2 | 4 | 8);

	iprintf("Synth!\r\n");

	SetAudioVolume(0xa0);
	PlayAudioWithCallback(AudioCallback, NULL);

	while (1) {
		if ((led++ % SPEED) == 0) {
			if (led >= SPEED * 4) led = 0;
			SetLEDs(1<<(led / SPEED));
		}
		Delay(10);
	}

	return 0;
}
