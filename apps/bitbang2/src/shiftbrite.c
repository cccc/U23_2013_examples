#include <stm32f4xx.h>
#include "shiftbrite.h"


// Convert r,g,b values to 32 bit integer value needed for the shiftbrites.
uint32_t convert_color(uint32_t r, uint32_t g, uint32_t b)
{
  if (r >= 1024) r = 1023;
  if (g >= 1024) g = 1023;
  if (b >= 1024) b = 1023;
  return (g<<0) | (r<<10) | (b<<20);
}


// Own wait function -- whoops
void wait_own(int v)
{
  for (int i=0; i<v; i++);
}



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



// Sets the latch pin (PIN 12) to high and then to low.
// At the same time also toggle the corresponding LED pins.
void latch(void)
{
  // Latch High
  GPIO_WriteBit(GPIOB, GPIO_Pin_12, SET); // Latch
  GPIO_WriteBit(GPIOD, GPIO_Pin_12, SET); // LED
  wait_own(wait_latch);

  // Latch Low
  GPIO_WriteBit(GPIOB, GPIO_Pin_12, RESET); // Latch
  GPIO_WriteBit(GPIOD, GPIO_Pin_12, RESET); // LED
  wait_own(wait_latch);
}
