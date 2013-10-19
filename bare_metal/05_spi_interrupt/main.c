// Implements SPI with interrupts
//
// The data is read from a uint32_t buffer, but the SPI sends uint16_t's at a
// time (max size). But as we need to send the uint32_t MSB (most significant
// bit) first, and the lower half of the uint32_t lies at the smaller address,
// we need to send the second half first. For convenience we flip the byteorder
// for the complete buffer (see 'rewrite_byteorder').


#include <stdio.h>
#include <stdlib.h>

#include <stm32f4xx.h>

#include "shiftbrite.h"
#include "spi.h"


#define wait_led 10000000


// Send buffer
uint32_t data_buffer[2];


int main()
{
  // Initialize pins
  init_leds();
  init_spi();


  // Init data buffer
  data_buffer[0] = convert_color(1023,1023,1023);
  data_buffer[1] = convert_color(512,512,512);

  // Change byteorder
  rewrite_byteorder(data_buffer, (uint16_t*) data_buffer, 2);

  // Write two 32 bit integers
  spi_write_buffer((uint16_t*) data_buffer, 2);
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
