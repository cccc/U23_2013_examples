#include <stm32f4xx.h>

#include "shiftbrite.h"
#include "dma_spi.h"

static uint32_t convenient_buffer __attribute__ ((section (".sram.bss")));


// Initialize SPI
void init_spi(void)
{
  // Enable clock for SPI hardware, GPIO port B, and DMA
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

  // SPI  (SCK PB13, MOSI PB15)
  GPIO_Init(GPIOB, &(GPIO_InitTypeDef){
      .GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15,
      .GPIO_Mode = GPIO_Mode_AF,
      .GPIO_Speed = GPIO_Speed_50MHz,
    });

  // Configure pins to be used by the SPI hardware (alternate function)
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

  // Latch pin
  GPIO_Init(GPIOB, &(GPIO_InitTypeDef){
      .GPIO_Pin = GPIO_Pin_12,
      .GPIO_Mode = GPIO_Mode_OUT,
      .GPIO_Speed = GPIO_Speed_50MHz,
    });

  // reset latch
  GPIO_WriteBit(GPIOB, GPIO_Pin_12, RESET);

  // Init SPI
  //
  // SPI can only work with 8 and 16 bit data packets, we use 16 to minimize
  // interrupts. This means we have to split up uint32_t's (-> sentUpperWord).
  SPI_Init(SPI2, &(SPI_InitTypeDef){
      .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8, // Configure Data speed
      .SPI_CPHA = SPI_CPHA_1Edge, // Sample data on rising edge
      .SPI_CPOL = SPI_CPOL_Low, // Clock is default low
      .SPI_CRCPolynomial = 1, // Don't use CRC
      .SPI_DataSize = SPI_DataSize_16b, // Send 16 bit words at a time
      .SPI_Direction = SPI_Direction_1Line_Tx, // Only enable sending (transmission/TX)
      .SPI_FirstBit = SPI_FirstBit_MSB, // Most Significant Bit first
      .SPI_Mode = SPI_Mode_Master, // STM32 is the master
      .SPI_NSS = SPI_NSS_Soft, // Don't use automatic chip select
    });


  // Tell the SPI hardware to request data from the DMA
  SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

  // Enable SPI hardware
  SPI_Cmd(SPI2, ENABLE);


  // Configure interrupts for the DMA (transfer complete)...
  NVIC_Init(&(NVIC_InitTypeDef){
      .NVIC_IRQChannel = DMA1_Stream4_IRQn,
      .NVIC_IRQChannelPreemptionPriority = 0,
      .NVIC_IRQChannelSubPriority = 1,
      .NVIC_IRQChannelCmd = ENABLE,
    });

  // ... and SPI (TXE, see ISR for explanation)
  NVIC_Init(&(NVIC_InitTypeDef){
      .NVIC_IRQChannel = SPI2_IRQn,
      .NVIC_IRQChannelPreemptionPriority = 0,
      .NVIC_IRQChannelSubPriority = 0,
      .NVIC_IRQChannelCmd = ENABLE,
    });

  // Configure the DMA temporarilly with 0-length buffer
  configure_dma(0, 0);

  // Enable the DMA TC (transmission complete) interrupt
  DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);
}

// Start the SPI/DMA, transmit 'data_count' uint16_t's
//
//  data_count in uint16_t units (2 byte)
void start_dma_spi(int data_count)
{
  // First reset DMA
  DMA_Cmd(DMA1_Stream4, DISABLE);

  // Set the current buffer length
  DMA_SetCurrDataCounter(DMA1_Stream4, data_count);

  // Reset Latch
  GPIO_WriteBit(GPIOB, GPIO_Pin_12, RESET);

  // Start DMA
  DMA_Cmd(DMA1_Stream4, ENABLE);
}

// Set which buffer the DMA will use, and its size.
//
//  buffer_size in uint16_t units (2 byte)
void configure_dma(uint16_t* buffer, int buffer_size)
{
  // Configure the DMA channel to move the right data in the right direction
  // DMA Channel 5 - SPI TX
  DMA_Init(DMA1_Stream4, &(DMA_InitTypeDef){
      .DMA_Channel = DMA_Channel_0,
      .DMA_BufferSize = buffer_size,
      .DMA_DIR = DMA_DIR_MemoryToPeripheral,
      .DMA_Memory0BaseAddr = (uint32_t)buffer,
      .DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,
      .DMA_MemoryInc = DMA_MemoryInc_Enable,
      .DMA_Mode = DMA_Mode_Normal,
      .DMA_PeripheralBaseAddr = (uint32_t)&SPI2->DR,
      .DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
      .DMA_PeripheralInc = DMA_PeripheralInc_Disable,
      .DMA_Priority = DMA_Priority_High,
    });
}

// Convenience function to write a uint32_t out, using an internal buffer.
void spi_write_int32(uint32_t data)
{
  // Write to internal buffer
  convenient_buffer = data;
  // Change byteorder
  rewrite_byteorder(&convenient_buffer, (uint16_t*) &convenient_buffer, 1);
  // Set to use the internal buffer
  configure_dma((uint16_t*) &convenient_buffer, 2);
  // Start the DMA/SPI
  start_dma_spi(2);
}



// DMA INTERRUPT


void spi_handleDMA1Ch4TcInterrupt(void);

void DMA1_Stream4_IRQHandler(void){
  // Check for interrupt cause (transmission complete)
  if(DMA_GetITStatus(DMA1_Stream4, DMA_IT_TCIF4) == SET)
  {
    spi_handleDMA1Ch4TcInterrupt();

    //Clear the DMA1 Stream4 Transfer Complete flag
    DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);
  }
}

// The DMA said 'transmission complete'. This means it just threw the last word
// into the SPI TX buffer. But the SPI is not done sending yet. So enable the
// TXE (transmit buffer empty) interrupt.
void spi_handleDMA1Ch4TcInterrupt(void){
  SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);
}



// SPI INTERRUPT


void spi_handleSPI2TxeInterrupt(void);

void SPI2_IRQHandler(void){
  // Check for interrupt cause (TX buffer empty)
  if(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == SET){
    spi_handleSPI2TxeInterrupt();
  }
}

// The SPI said 'TX buffer empty'. However it is not done sending yet. It still
// is shifting data bits out of its internal shift register. So wait until it is
// done finally.
void spi_handleSPI2TxeInterrupt(void){
  // Check whether the SPI is _really_ done
  if(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == RESET){

    // Disable the TXE interrupt
    SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);

    // Do a latch
    latch();
  }
}
