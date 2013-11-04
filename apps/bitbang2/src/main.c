// Implements SPI by GPIO bitbanging
//
// Simultaneously uses the SPI and the LED pins, so one can see what happens.
// For that comment in the lines in 'shiftbrite.h' and 'bitbanging.h'


#include <stdio.h>
#include <stdlib.h>

#include <stm32f4xx.h>

#include "shiftbrite.h"
#include "bitbanging.h"


#define wait_led 10000000


// Send buffer
uint32_t data_buffer[2];


int main()
{
  // Initialize pins
  init_leds();
  init_gpio();


  // Init data buffer
  data_buffer[0] = convert_color(1023,1023,1023);
  data_buffer[1] = convert_color(512,512,512);

  // Write two 32 bit integers
  gpiospi_write_buffer(data_buffer, 2);
  // Do a latch to write data value into the chips
  latch();
  // wait a bit
  wait_own(wait_led);

  // Write some more colors
  gpiospi_write_int32(convert_color(1023,0,0));
  latch();
  wait_own(wait_led);

  gpiospi_write_int32(convert_color(0,1023,0));
  latch();
  wait_own(wait_led);

  gpiospi_write_int32(convert_color(0,0,1023));
  latch();
  wait_own(wait_led);

  gpiospi_write_int32(convert_color(0,0,0));
  latch();


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
