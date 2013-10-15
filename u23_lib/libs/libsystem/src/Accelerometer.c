#include <Accelerometer.h>

#include <stm32f4xx.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>

#include <SysTick.h>

static uint8_t ReadByte(uint8_t address);
static void ReadBytes(uint8_t *buffer,uint8_t address,int numbytes);
static void WriteByte(uint8_t byte,uint8_t address);
static void WriteBytes(uint8_t *bytes,uint8_t address,int numbytes);

static inline void LowerCS();
static inline void RaiseCS();
static uint8_t TransferByte(uint8_t byte);
static void LIS302DL_LowLevel_Init(void);

static int8_t calibrationVector[3];

void InitializeAccelerometer(void)
{
	LIS302DL_LowLevel_Init();

	SetAccelerometerMainConfig(
		LIS302DL_LOWPOWERMODE_ACTIVE|
		LIS302DL_DATARATE_100|
		LIS302DL_XYZ_ENABLE|
		LIS302DL_FULLSCALE_2_3|
		LIS302DL_SELFTEST_NORMAL);

	//Wait one second for data to stabilize
	Delay(100);

	SetAccelerometerFilterConfig(
		LIS302DL_FILTEREDDATASELECTION_BYPASSED|
		LIS302DL_HIGHPASSFILTER_LEVEL_1|
		LIS302DL_HIGHPASSFILTERINTERRUPT_1_2);

}

static void LIS302DL_LowLevel_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

  /* Enable the SPI periph */
  RCC_APB2PeriphClockCmd(LIS302DL_SPI_CLK, ENABLE);

  /* Enable SCK, MOSI and MISO GPIO clocks */
  RCC_AHB1PeriphClockCmd(LIS302DL_SPI_SCK_GPIO_CLK | LIS302DL_SPI_MISO_GPIO_CLK | LIS302DL_SPI_MOSI_GPIO_CLK, ENABLE);

  /* Enable CS  GPIO clock */
  RCC_AHB1PeriphClockCmd(LIS302DL_SPI_CS_GPIO_CLK, ENABLE);

  /* Enable INT1 GPIO clock */
  RCC_AHB1PeriphClockCmd(LIS302DL_SPI_INT1_GPIO_CLK, ENABLE);

  /* Enable INT2 GPIO clock */
  RCC_AHB1PeriphClockCmd(LIS302DL_SPI_INT2_GPIO_CLK, ENABLE);

  GPIO_PinAFConfig(LIS302DL_SPI_SCK_GPIO_PORT, LIS302DL_SPI_SCK_SOURCE, LIS302DL_SPI_SCK_AF);
  GPIO_PinAFConfig(LIS302DL_SPI_MISO_GPIO_PORT, LIS302DL_SPI_MISO_SOURCE, LIS302DL_SPI_MISO_AF);
  GPIO_PinAFConfig(LIS302DL_SPI_MOSI_GPIO_PORT, LIS302DL_SPI_MOSI_SOURCE, LIS302DL_SPI_MOSI_AF);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  /* SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = LIS302DL_SPI_SCK_PIN;
  GPIO_Init(LIS302DL_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /* SPI  MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  LIS302DL_SPI_MOSI_PIN;
  GPIO_Init(LIS302DL_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /* SPI MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin = LIS302DL_SPI_MISO_PIN;
  GPIO_Init(LIS302DL_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /* SPI configuration -------------------------------------------------------*/
  SPI_I2S_DeInit(LIS302DL_SPI);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_Init(LIS302DL_SPI, &SPI_InitStructure);

  /* Enable SPI1  */
  SPI_Cmd(LIS302DL_SPI, ENABLE);

  /* Configure GPIO PIN for Lis Chip select */
  GPIO_InitStructure.GPIO_Pin = LIS302DL_SPI_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(LIS302DL_SPI_CS_GPIO_PORT, &GPIO_InitStructure);

  /* Deselect : Chip Select high */
  GPIO_SetBits(LIS302DL_SPI_CS_GPIO_PORT, LIS302DL_SPI_CS_PIN);

  /* Configure GPIO PINs to detect Interrupts */
  GPIO_InitStructure.GPIO_Pin = LIS302DL_SPI_INT1_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(LIS302DL_SPI_INT1_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LIS302DL_SPI_INT2_PIN;
  GPIO_Init(LIS302DL_SPI_INT2_GPIO_PORT, &GPIO_InitStructure);
}


uint8_t PingAccelerometer(void)
{
	uint8_t byte=ReadByte(LIS302DL_WHO_AM_I_ADDR);
	return byte==0x3b;
}

void SetAccelerometerMainConfig(uint8_t config)
{
	WriteByte(config,LIS302DL_CTRL_REG1_ADDR);
}

void SetAccelerometerFilterConfig(uint8_t config)
{
	WriteByte(config,LIS302DL_CTRL_REG2_ADDR);
}

void SetAccelerometerInterruptConfig(uint8_t config)
{
	WriteByte(config,LIS302DL_CLICK_CFG_REG_ADDR);
}

void CalibrateAccelerometer(void) {
	ReadRawAccelerometerData(calibrationVector);
}

void ReadCalibratedAccelerometerData(int8_t values[3]) {
	int8_t newValues[3];
	ReadRawAccelerometerData(newValues);

	values[0]=(int8_t)newValues[0]-calibrationVector[0];
	values[1]=(int8_t)newValues[1]-calibrationVector[1];
	values[2]=(int8_t)newValues[2]-calibrationVector[2];
}

void ResetAccelerometer(void)
{
	uint8_t val=ReadByte(LIS302DL_CTRL_REG2_ADDR);
	WriteByte(val|LIS302DL_BOOT_REBOOTMEMORY,LIS302DL_CTRL_REG2_ADDR);
}

void ResetAccelerometerFilter(void)
{
	ReadByte(LIS302DL_HP_FILTER_RESET_REG_ADDR);
}

void ReadRawAccelerometerData(int8_t values[3])
{
	uint8_t buffer[5];
	ReadBytes(buffer,LIS302DL_OUT_X_ADDR,5);
	values[0]=(int8_t)buffer[0];
	values[1]=(int8_t)buffer[2];
	values[2]=(int8_t)buffer[4];
}

#define ReadCommand 0x80
#define MultiByteCommand 0x40

static uint8_t ReadByte(uint8_t address)
{
	LowerCS();

	TransferByte(address|ReadCommand);
	uint8_t byte=TransferByte(0);

	RaiseCS();

	return byte;
}

static void ReadBytes(uint8_t *buffer,uint8_t address,int numbytes)
{
	if(numbytes>1) address|=ReadCommand|MultiByteCommand;
	else address|=ReadCommand;

	LowerCS();

	TransferByte(address);
	for(int i=0;i<numbytes;i++) buffer[i]=TransferByte(0);

	RaiseCS();
}

static void WriteByte(uint8_t byte,uint8_t address)
{
	LowerCS();

	TransferByte(address);
	TransferByte(byte);

	RaiseCS();
}

static void WriteBytes(uint8_t *bytes,uint8_t address,int numbytes)
{
	if(numbytes>1) address|=MultiByteCommand;

	LowerCS();

	TransferByte(address);
	for(int i=0;i<numbytes;i++) TransferByte(bytes[i]);

	RaiseCS();
}

static inline void LowerCS()
{
	GPIO_ResetBits(LIS302DL_SPI_CS_GPIO_PORT, LIS302DL_SPI_CS_PIN);
}

static inline void RaiseCS()
{
	GPIO_SetBits(LIS302DL_SPI_CS_GPIO_PORT, LIS302DL_SPI_CS_PIN);
}

#define Timeout 0x1000

static uint8_t TransferByte(uint8_t byte)
{
	uint32_t timer=Timeout;
	while(!(SPI1->SR&SPI_SR_TXE)) if(timer--==0) return 0;

	SPI1->DR=byte;

	timer=Timeout;
	while(!(SPI1->SR&SPI_SR_RXNE)) if(timer--==0) return 0;

	return SPI1->DR;
}
