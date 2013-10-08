#include <Random.h>
#include <stm32f4xx.h>
#include <stm32f4xx_rng.h>

static int16_t RNGInitialized = 0;

void InitializeRandom(void) {
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);
	RNG_ITConfig(DISABLE);
	RNGInitialized = 1;
}

uint32_t GetRandomInteger(void) {
	if (!RNGInitialized) {
		//ERROR
		return 0;
	}
	while (RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET);
	return RNG_GetRandomNumber();
}

void DeInitializeRandom(void) {
	RNG_Cmd(DISABLE);
	RNG_DeInit();
	RNGInitialized = 0;
}
