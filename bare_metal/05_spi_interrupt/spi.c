#include <stm32f4xx.h>
#include "shiftbrite.h"
#include "spi.h"


// SPI
void init_spi_general(void);
void init_spi(void); //TODO
void spi_write_int32(uint32_t data);
void wait_spi_done(void);


static uint16_t spiTxCounter;
static volatile uint8_t spiBusyFlag;
static uint16_t* spiDataBuffer;
static uint16_t spiDataBuffer_length;

static uint32_t convenient_buffer;



// Initialize SPI
void init_spi(void)
{
  // Enable clock for SPI hardware and GPIO port B
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
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
  // interrupts. This means we have to split up uint32_t's. As the byte order of
  // the chip is wrong, there is the function 'rewrite_byteorder', which takes
  // a uint32_t buffer and converts it into a uint16_t buffer with the right
  // byte order.
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

  // Enable SPI interrupt
  NVIC_Init(&(NVIC_InitTypeDef){
      .NVIC_IRQChannel = SPI2_IRQn,
      .NVIC_IRQChannelPreemptionPriority = 0,
      .NVIC_IRQChannelSubPriority = 0,
      .NVIC_IRQChannelCmd = ENABLE,
    });

  // Enable SPI hardware
  SPI_Cmd(SPI2, ENABLE);

  // init internal state
  spiBusyFlag = 0;
}



void spi_handleSPI2Interrupt(void);

// This is the SPI interrupt handler
// we call a separate function
void SPI2_IRQHandler(void){
  spi_handleSPI2Interrupt();
}

// Enable/Disable firing of the TXE (transmit buffer empty) interrupt
void spi_enableTxInterrupt(void){
  SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);
}
void spi_disableTxInterrupt(void){
  SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
}

// Wait until the SPI is ready to accept a new buffer
void wait_spi_ready(void){
  while(spiBusyFlag);
}
// Return whether SPI is ready to accept a new buffer
uint8_t is_spi_busy(void){
  return spiBusyFlag;
}

// Convenience function to write a single uint32_t through SPI,
// using an internal buffer
void spi_write_int32(uint32_t data)
{
  convenient_buffer = data;
  rewrite_byteorder(&convenient_buffer, (uint16_t*) &convenient_buffer, 1);
  spi_write_buffer((uint16_t*) &convenient_buffer, 2);
}

// Write a complete buffer through SPI, block until ready
//
//  data_count in uint16_t units (2 byte)
void spi_write_buffer(uint16_t* data, int data_count)
{
  wait_spi_ready();
  spiBusyFlag = 1;
  spiTxCounter = 0;
  spiDataBuffer_length = data_count;
  spiDataBuffer = data;

  // Enable the TXE interrupt
  // it should immediately fire and then start transmitting data
  spi_enableTxInterrupt();
}

void spi_handleSPI2Interrupt(void){
  // Check for interrupt cause (Transmit Buffer Empty)
  if(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == SET){
    // The transmit buffer is empty, we can queue another word

    if (spiTxCounter >= spiDataBuffer_length) {

      // We have sent the complete buffer, but the SPI may still be transmitting
      // So check if the SPI is _really_ done
      if (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == RESET)
      {
        // Disable the interrupt
        spi_disableTxInterrupt();

        // Do a latch to write data value into the chips
        latch();

        // Tell everyone else that we are done
        spiBusyFlag = 0;
      }

    } else {
      // Send the next uint16_t
      SPI_I2S_SendData(SPI2, spiDataBuffer[spiTxCounter++]);
    }
  }
}
