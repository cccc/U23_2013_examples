#ifndef DMA_SPI_H_QGT9AJSD
#define DMA_SPI_H_QGT9AJSD


void init_spi(void);
void configure_dma(uint16_t* buffer, int buffer_size);
void start_dma_spi(int data_count);
void spi_write_int32(uint32_t data);


#endif /* end of include guard: DMA_SPI_H_QGT9AJSD */

