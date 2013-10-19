// Implements SPI with interrupts


#include <stdio.h>
#include <stdlib.h>

#include <stm32f4xx.h>

#include "shiftbrite.h"
#include "dma_spi.h"


#define wait_led 10000000


// Send buffer
// Make sure to always use this attribute. It moves the variable into the sram,
// which is the only section the DMA can access.
static uint32_t data_buffer[4] __attribute__ ((section (".sram.bss")));


int main()
{
  // Initialize pins
  init_leds();
  init_spi();

  // Configure which buffer the dma should use
  configure_dma((uint16_t*) data_buffer, 4);


  // Init data buffer
  data_buffer[0] = convert_color(1023,1023,1023);
  data_buffer[1] = convert_color(1023,0,0);
  data_buffer[2] = convert_color(0,1023,0);
  data_buffer[3] = convert_color(0,0,1023);

  // Change byteorder, see 05_spi_interrupt
  rewrite_byteorder(data_buffer, (uint16_t*) data_buffer, 4);

  // Write two 32 bit integers
  start_dma_spi(2*4);
  // wait a bit
  wait_own(wait_led);

  // Write some more colors
  spi_write_int32(convert_color(1023,0,0));
  wait_own(wait_led);

  spi_write_int32(convert_color(0,1023,0));
  wait_own(wait_led);

  spi_write_int32(convert_color(0,0,1023));
  wait_own(wait_led);

  spi_write_int32(convert_color(0,0,0));


  // Set end notification LED
  GPIO_SetBits(GPIOD, GPIO_Pin_14);

  while(1);
}


#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
	ERROR_MSG("Assertion failed in file %s:%d\r\n", file, line);
	while(1);	
}
#endif
