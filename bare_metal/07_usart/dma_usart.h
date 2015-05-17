#ifndef DMA_USART_H_QGT9AJSD
#define DMA_USART_H_QGT9AJSD

#include <stdint.h>

void init_usart(void);
void start_dma_usart(int data_count);
void configure_dma_usart(uint8_t* buffer, int buffer_size);


#endif /* end of include guard: DMA_USART_H_QGT9AJSD */

