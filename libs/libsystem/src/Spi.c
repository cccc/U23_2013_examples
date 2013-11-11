#include <stm32f4xx.h>

#include "Spi.h"

#define NULL 0


static volatile uint8_t spi_running = 0;
static volatile uint8_t spi_txe = 1;
static volatile uint8_t spi_last_byte = 0;

static uint8_t* spi_transmit_buffer = NULL;
static uint8_t* spi_receive_buffer = NULL;
static uint16_t spi_data_buffersize = 0;
static volatile uint16_t spi_data_rx_counter = 0;
static volatile uint16_t spi_data_tx_counter = 0;
static void (*spi_external_handler)(void);


// A function to allow bypassing of the here used SPI2_IRQHandler.
// Call spi_overwrite_interrupt_hanlder(your_handler) and it will feel as if
// your handler was the SPI2_IRQHandler.
// To reset call with a value of 0.
void spi_overwrite_interrupt_handler(void (*handler)(void))
{
  spi_external_handler = handler;
}


// Initialize SPI hardware
uint8_t spi_initialize(void)
{
  // Enable clock for SPI hardware and GPIO port B
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

  // SPI  (SCK PB13, MISO PB14, MOSI PB15)
  GPIO_Init(GPIOB, &(GPIO_InitTypeDef){
      .GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15,
      .GPIO_Mode = GPIO_Mode_AF,
      .GPIO_Speed = GPIO_Speed_50MHz,
    });

  // Configure pins to be used by the SPI hardware (alternate function)
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

  // Init SPI
  SPI_Init(SPI2, &(SPI_InitTypeDef){
      .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64, // Configure Data speed
      .SPI_CPHA = SPI_CPHA_1Edge, // Sample data on rising edge
      .SPI_CPOL = SPI_CPOL_Low, // Clock is default low
      .SPI_CRCPolynomial = 1, // Don't use CRC
      .SPI_DataSize = SPI_DataSize_8b, // Send 16 bit words at a time
      .SPI_Direction = SPI_Direction_2Lines_FullDuplex, // Only enable sending (transmission/TX)
      .SPI_FirstBit = SPI_FirstBit_MSB, // Most Significant Bit first
      .SPI_Mode = SPI_Mode_Master, // STM32 is the master
      .SPI_NSS = SPI_NSS_Soft, // Don't use automatic chip select
    });

  // Enable SPI interrupt
  NVIC_Init(&(NVIC_InitTypeDef){
      .NVIC_IRQChannel = SPI2_IRQn,
      .NVIC_IRQChannelPreemptionPriority = 0,
      .NVIC_IRQChannelSubPriority = 0,
      .NVIC_IRQChannelCmd = ENABLE,
    });

  // Enable SPI hardware
  SPI_Cmd(SPI2, ENABLE);
}

// Return wheter the SPI is done completely
uint8_t spi_is_running(void)
{
  return spi_running;
}

// Return wheter the SPI transmit buffer is empty, and can take more data
uint8_t spi_is_txe(void)
{
  return spi_txe;
}

// Wait for spi_is_running == false
void spi_wait_running(void)
{
  while (spi_running);
}

// Wait for spi_is_running == true
void spi_wait_txe(void)
{
  while (!spi_txe);
}

// Put a byte into the transmit buffer
void spi_send_byte(uint8_t data)
{
  spi_wait_txe();
  spi_transmit_buffer = NULL;
  spi_receive_buffer = NULL;
  spi_txe = 0;
  spi_running = 1;
  SPI_I2S_SendData(SPI2, data);
  SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
}

// Same as spi_send_byte(0) - for receive-only applications
void spi_receive_byte(void)
{
  spi_send_byte(0);
}

// Get the last received byte
uint8_t spi_get_last_byte(void)
{
  return spi_last_byte;
}


// Send/receive count bytes of data. Any of the buffers may be NULL. Then
// there will be sent bytes of value 0 / received bytes won't be saved.
void spi_send_buffer(uint8_t* send_buffer, uint8_t* receive_buffer, uint16_t count)
{
  spi_wait_running();
  spi_transmit_buffer = send_buffer;
  spi_receive_buffer = receive_buffer;
  spi_data_buffersize = count;
  spi_data_tx_counter = 0;
  spi_data_rx_counter = 0;
  spi_txe = 0;
  spi_running = 1;
  SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);
  SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
}



void SPI2_IRQHandler(void){
  // If the external interrupt handler is set, call it
  if (spi_external_handler)
  {
    spi_external_handler();
    return;
  }

  // Check for interrupt cause
  if(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == SET)
  {
    spi_last_byte = SPI_I2S_ReceiveData(SPI2);
    if (spi_receive_buffer)
    {
      // buffered mode
      spi_receive_buffer[spi_data_rx_counter++] = spi_last_byte;
      if (spi_data_rx_counter >= spi_data_buffersize)
      {
        spi_receive_buffer = NULL;
        spi_running = 0;
        SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
      }
    } else {
      // non buffered mode
      spi_running = 0;
      SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
    }
  }

  else if(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == SET)
  {
    // only buffered mode
    if (spi_data_tx_counter >= spi_data_buffersize)
    {
      SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
      spi_transmit_buffer = NULL;
      spi_txe = 1;
    } else {
      SPI_I2S_SendData(SPI2, spi_transmit_buffer ? spi_transmit_buffer[spi_data_tx_counter] : 0);
      spi_data_tx_counter++;
    }
  }
}
