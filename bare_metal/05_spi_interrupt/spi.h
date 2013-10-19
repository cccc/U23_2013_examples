#ifndef SPI_H_VJY8LF04
#define SPI_H_VJY8LF04


void init_spi(void);
void wait_spi_ready(void);
//void wait_spi_done(void); // TODO (?)
uint8_t is_spi_busy(void);
void spi_write_int32(uint32_t data);
void spi_write_buffer(uint16_t* data, int data_count);


#endif /* end of include guard: SPI_H_VJY8LF04 */

