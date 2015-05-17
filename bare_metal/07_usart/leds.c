#include <stm32f4xx.h>

#include "leds.h"

// Initialize LED pins
void init_leds(void)
{
  // Enable LED port clock
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  // Init pins
  GPIO_Init(GPIOD, &(GPIO_InitTypeDef){
      .GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15,
      .GPIO_Mode = GPIO_Mode_OUT,
      .GPIO_PuPd = GPIO_PuPd_UP,
      .GPIO_Speed = GPIO_Speed_50MHz,
    });

  // Reset all LED pins
  GPIO_WriteBit(GPIOD, GPIO_Pin_12, RESET);
  GPIO_WriteBit(GPIOD, GPIO_Pin_13, RESET);
  GPIO_WriteBit(GPIOD, GPIO_Pin_14, RESET);
  GPIO_WriteBit(GPIOD, GPIO_Pin_15, RESET);
}
