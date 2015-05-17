// Implements USART with DMA


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <stm32f4xx.h>

#include "leds.h"
#include "dma_usart.h"


// Send buffer
// Make sure to always use this attribute. It moves the variable into the sram,
// which is the only section the DMA can access.
static uint8_t data_buffer[100] __attribute__ ((section (".sram.bss")));


int main()
{
  // Initialize pins
  init_leds();
  init_usart();

  memcpy(data_buffer, "Hello World!", 13);
  configure_dma_usart(data_buffer, 0);
  start_dma_usart(13);

  while(1);
}


void usart_txc_callback(void)
{
}


#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{
	ERROR_MSG("Assertion failed in file %s:%d\r\n", file, line);
	while(1);	
}
#endif
