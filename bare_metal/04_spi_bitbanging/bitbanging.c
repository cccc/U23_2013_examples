#include <stm32f4xx.h>
#include "shiftbrite.h"
#include "bitbanging.h"


// GPIO SPI
void init_gpio(void)
{
  // Enable Clock
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  // SPI  (SCK PB13, MOSI PB15)
  GPIO_Init(GPIOB, &(GPIO_InitTypeDef){
      .GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15,
      .GPIO_Mode = GPIO_Mode_OUT,
      .GPIO_Speed = GPIO_Speed_50MHz,
    });

  // Latch
  GPIO_Init(GPIOB, &(GPIO_InitTypeDef){
      .GPIO_Pin = GPIO_Pin_12,
      .GPIO_Mode = GPIO_Mode_OUT,
      .GPIO_Speed = GPIO_Speed_50MHz,
    });

  // Reset Latch
  GPIO_WriteBit(GPIOB, GPIO_Pin_12, RESET);
}


// Write a complete data buffer to spi out using bitbanging.
void gpiospi_write_buffer(uint32_t* data, int data_count)
{
  for (int i=0; i<data_count; i++)
    gpiospi_write_int32(data[i]);
}


// Write a single 32 bit integer out to SPI by bitbanging.
// Also toggles the corresponding LED pins.
void gpiospi_write_int32(uint32_t data)
{
  // Go through the bits, MSB first
  for (int i=31; i>=0; i--)
  {
    // Get current bit
    uint32_t bit = data & (0x1<<i);

    // Set Data
    GPIO_WriteBit(GPIOB, GPIO_Pin_15, bit ? SET : RESET); // Data
    GPIO_WriteBit(GPIOD, GPIO_Pin_15, bit ? SET : RESET); // LED
    wait_own(wait_data);

    // Set clock high
    GPIO_WriteBit(GPIOB, GPIO_Pin_13, SET); // Clock
    GPIO_WriteBit(GPIOD, GPIO_Pin_13, SET); // LED
    wait_own(wait_data);

    // Set clock low
    GPIO_WriteBit(GPIOB, GPIO_Pin_13, RESET); // Clock
    GPIO_WriteBit(GPIOD, GPIO_Pin_13, RESET); // LED
    wait_own(wait_data);
  }
}
