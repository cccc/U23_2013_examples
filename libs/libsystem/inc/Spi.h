#ifndef SPI_H_FSTYHOEP
#define SPI_H_FSTYHOEP


/*
 * Implements a high level interface to the SPI hardware. Though this will
 * probably be too unflexible for practically any application.
 *
 * USAGE non-buffered version:
 *  1) Init
 *  2) Do you want to transmit or receive?
 *    a) Transmit only:
 *        Call spi_send_byte. The SPI will start sending. If you want
 *        to send more data, wait until the transmit buffer is empty
 *        (spi_wait_txe) and send the next byte.
 *    b) Transmit and receive:
 *        Call spi_send_byte. Then wait until the SPI is done
 *        (spi_wait_running) and read the received byte with
 *        spi_get_last_byte.
 *    c) Receive only:
 *        Same as b). You have to send some data (e.g. 0), which is
 *        what spi_receive_byte does.
 *
 * USAGE buffered version:
 *  1) Init
 *  2) Start transfer:
 *      Call spi_send_buffer with your transmit and/or receive buffer and
 *      the data count.
 *  3) Wait until finished
 *
 * USAGE own SPI code:
 *  1) Do everything as usual
 *  2) Instead of implementing SPI2_IRQHandler, write a different function, and
 *      set it to be called with spi_overwrite_interrupt_handler(your_handler)
 */


// Bypass used SPI2_IRQHandler
void spi_overwrite_interrupt_handler(void (*handler)(void));
// Initialize SPI hardware
uint8_t spi_initialize(void);
// Return wheter the SPI is done completely
uint8_t spi_is_running(void);
// Return wheter the SPI transmit buffer is empty, and can take more data
uint8_t spi_is_txe(void);
// Wait for spi_is_running == false
void spi_wait_running(void);
// Wait for spi_is_running == true
void spi_wait_txe(void);
// Put a byte into the transmit buffer
void spi_send_byte(uint8_t data);
// Same as spi_send_byte(0) - for receive-only applications
void spi_receive_byte(void);
// Get the last received byte
uint8_t spi_get_last_byte(void);

// Send/receive count bytes of data. Any of the buffers may be NULL. Then
// there will be sent bytes of value 0 / received bytes won't be saved.
void spi_send_buffer(uint8_t* send_buffer, uint8_t* receive_buffer, uint16_t count);


#endif /* end of include guard: SPI_H_FSTYHOEP */
