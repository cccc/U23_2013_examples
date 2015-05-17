#include <stm32f4xx.h>

#include "dma_usart.h"


// Initialize SPI
void init_usart(void)
{
  // Enable clock for SPI hardware, GPIO port B, and DMA
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

  // USART1  (TX PA9)
  GPIO_Init(GPIOA, &(GPIO_InitTypeDef){
      .GPIO_Pin = GPIO_Pin_9,
      .GPIO_Mode = GPIO_Mode_AF,
      .GPIO_Speed = GPIO_Speed_50MHz,
    });

  // Configure pins to be used by the USART hardware (alternate function)
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);

  // Init USART
  USART_Init(USART1, &(USART_InitTypeDef){
      // baudrate: 9600
      // clock: 168 Mhz with prescaler=2  ->  PCLK = 84 Mhz
      // oversample by 16  ->  USARTDIV = 84 Mhz / (16 * baudrate) = 546.875 = 546 + (14/16)
      .USART_BaudRate = (546<<4) | (14<<0),
      .USART_WordLength = USART_WordLength_8b,
      .USART_StopBits = USART_StopBits_1,
      .USART_Parity = USART_Parity_No,
      .USART_Mode = USART_Mode_Tx,
      .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    });


  // Tell the USART hardware to request data from the DMA
  USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);

  // Enable USART hardware
  USART_Cmd(USART1, ENABLE);


  // Configure interrupts for the DMA (transfer complete)...
  NVIC_Init(&(NVIC_InitTypeDef){
      .NVIC_IRQChannel = DMA2_Stream7_IRQn,
      .NVIC_IRQChannelPreemptionPriority = 0,
      .NVIC_IRQChannelSubPriority = 1,
      .NVIC_IRQChannelCmd = ENABLE,
    });

  // ... and USART (TXC)
  NVIC_Init(&(NVIC_InitTypeDef){
      .NVIC_IRQChannel = USART1_IRQn,
      .NVIC_IRQChannelPreemptionPriority = 0,
      .NVIC_IRQChannelSubPriority = 0,
      .NVIC_IRQChannelCmd = ENABLE,
    });

  // Configure the DMA temporarilly with 0-length buffer
  configure_dma_usart(0, 0);

  // Enable the DMA TC (transmission complete) interrupt
  DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);
}

// Start the USART/DMA, transmit 'data_count' uint8_t's
void start_dma_usart(int data_count)
{
  // First reset DMA
  DMA_Cmd(DMA2_Stream7, DISABLE);

  // Set the current buffer length
  DMA_SetCurrDataCounter(DMA2_Stream7, data_count);

  // Start DMA
  DMA_Cmd(DMA2_Stream7, ENABLE);
}

// Set which buffer the DMA will use, and its size.
void configure_dma_usart(uint8_t* buffer, int buffer_size)
{
  // Configure the DMA channel to move the right data in the right direction
  // DMA Channel 4 - USART1 TX
  DMA_Init(DMA2_Stream7, &(DMA_InitTypeDef){
      .DMA_Channel = DMA_Channel_4,
      .DMA_BufferSize = buffer_size,
      .DMA_DIR = DMA_DIR_MemoryToPeripheral,
      .DMA_Memory0BaseAddr = (uint32_t)buffer,
      .DMA_MemoryDataSize = DMA_MemoryDataSize_Byte,
      .DMA_MemoryInc = DMA_MemoryInc_Enable,
      .DMA_Mode = DMA_Mode_Normal,
      .DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR,
      .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
      .DMA_PeripheralInc = DMA_PeripheralInc_Disable,
      .DMA_Priority = DMA_Priority_High,
    });
}


// DMA INTERRUPT


void DMA2_Stream7_IRQHandler(void){
  // Check for interrupt cause (transmission complete)
  if(DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) == SET)
  {
    USART_ITConfig(USART1, USART_IT_TC, ENABLE);

    //Clear the DMA1 Stream4 Transfer Complete flag
    DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
  }
}


// USART INTERRUPT


void usart_txc_callback(void);

void USART1_IRQHandler(void){
  // Check for interrupt cause (TX buffer empty)
  if(USART_GetFlagStatus(USART1, USART_FLAG_TC) == SET){
    // really done sending
    usart_txc_callback();
  }
}
