#include <stm32f4xx.h>
#include <Audio.h>
#include <SysTick.h>

#include <stdlib.h>
#include <stdbool.h>

static void WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue);
static uint32_t ReadRegister(uint8_t RegisterAddr);
static void StartAudioDMAAndRequestBuffers();
static void StopAudioDMA();
static void ProvideAudioBuffer(void *samples, int numsamples);
static bool ProvideAudioBufferWithoutBlocking(void *samples, int numsamples);


static _Bool audio_initialized = 0;
static AudioCallbackFunction *CallbackFunction;
static void *CallbackContext;
static uint32_t Frequency;
static int16_t * volatile NextBufferSamples;
static volatile int NextBufferLength;
static volatile bool DMARunning;

static volatile int BufferNumber;
static int16_t SampleBuffer[2][256] __attribute__((section (".sram.bss")));

void InitializeAudio(uint32_t freq)
{
	//FIXME: Original code in libgaming of U23 2012 had a check here if the Filesystem was initialized
	//The SD-Card interface uses a few pins connected to the audio codec as well
	if(audio_initialized)
		return;

	// Intitialize state.
	audio_initialized = 1;
	CallbackFunction = NULL;
	CallbackContext = NULL;
	NextBufferSamples = NULL;
	NextBufferLength = 0;
	BufferNumber = 0;
	DMARunning = false;
	Frequency = freq;

	// Turn on peripherals.
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |
			RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD |
			RCC_AHB1Periph_DMA1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1 | RCC_APB1Periph_SPI3,
			ENABLE);

	// Configure reset pin.
	GPIO_Init(GPIOD, &(GPIO_InitTypeDef){
		.GPIO_Pin = GPIO_Pin_4,
		.GPIO_Mode = GPIO_Mode_OUT,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_NOPULL,
		.GPIO_Speed = GPIO_Speed_50MHz
	});

	// Configure I2C SCL and SDA pins.
	GPIO_Init(GPIOB, &(GPIO_InitTypeDef){
		.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_9,
		.GPIO_Mode = GPIO_Mode_AF,
		.GPIO_OType = GPIO_OType_OD,
		.GPIO_PuPd = GPIO_PuPd_NOPULL,
		.GPIO_Speed = GPIO_Speed_50MHz
	});
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);

	// Configure I2S MCK, SCK, SD pins.
	GPIO_Init(GPIOC, &(GPIO_InitTypeDef){
		.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_10 | GPIO_Pin_12,
		.GPIO_Mode = GPIO_Mode_AF,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_NOPULL,
		.GPIO_Speed = GPIO_Speed_50MHz
	});
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);

	// Configure I2S WS pin.
	GPIO_Init(GPIOA, &(GPIO_InitTypeDef){
		.GPIO_Pin = GPIO_Pin_4,
		.GPIO_Mode = GPIO_Mode_AF,
		.GPIO_OType = GPIO_OType_PP,
		.GPIO_PuPd = GPIO_PuPd_NOPULL,
		.GPIO_Speed = GPIO_Speed_50MHz
	});
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI3);

	// Reset the codec.
	GPIO_ResetBits(GPIOD, GPIO_Pin_4);
	Delay(0x10);
	GPIO_SetBits(GPIOD, GPIO_Pin_4);

	// Reset I2C.
	I2C_DeInit(I2C1);

	// Configure I2C.
	I2C_Cmd(I2C1, ENABLE);
	I2C_Init(I2C1, &(I2C_InitTypeDef){
		.I2C_ClockSpeed = 100000,
		.I2C_Mode = I2C_Mode_I2C,
		.I2C_DutyCycle = I2C_DutyCycle_2,
		.I2C_OwnAddress1 = 0x33,
		.I2C_Ack = I2C_Ack_Enable,
		.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit
	});

	// Configure codec.
	WriteRegister(0x02, 0x01); // Keep codec powered off.

	// Start "required" initialization (DOCS)
	WriteRegister(0x00, 0x99);
	WriteRegister(0x47, 0x80);
	WriteRegister(0x32, 1<<7);
	WriteRegister(0x32, 0x00);
	WriteRegister(0x00, 0x00);
	// End

	WriteRegister(0x04, 0xaf); // SPK always off and HP always on.

	WriteRegister(0x05, 0x81); // Clock configuration: Auto detection.
	WriteRegister(0x06, 0x04); // Set slave mode and Philips audio standard.

	SetAudioVolume(0xff);

	// Power on the codec.
	WriteRegister(0x02, 0x9e);

	// Configure codec for fast shutdown.
	WriteRegister(0x0a, 0x00); // Disable the analog soft ramp.
	WriteRegister(0x0e, 0x04); // Disable the digital soft ramp.

	WriteRegister(0x27, 0x00); // Disable the limiter attack level.
	WriteRegister(0x1f, 0x0f); // Adjust bass and treble levels.

	WriteRegister(0x1a, 0x0a); // Adjust PCM volume level.
	WriteRegister(0x1b, 0x0a);

	// Reset I2S.
	SPI_I2S_DeInit(SPI3);

	I2S_Init(SPI3, &(I2S_InitTypeDef){
		.I2S_Mode = I2S_Mode_MasterTx,
		.I2S_Standard = I2S_Standard_Phillips,
		.I2S_DataFormat = I2S_DataFormat_16b,
		.I2S_MCLKOutput = I2S_MCLKOutput_Enable,
		.I2S_AudioFreq = Frequency,
		.I2S_CPOL = I2S_CPOL_Low
	});
	I2S_Cmd(SPI3, ENABLE);
}

void DeinitializeAudio(void)
{
	if (!audio_initialized) return;
	I2S_Cmd(SPI3, DISABLE);
	SPI_I2S_DeInit(SPI3);
	I2C_Cmd(I2C1, DISABLE);
	I2C_DeInit(I2C1);

	audio_initialized = 0;
}

void AudioOn(void)
{
	if (!audio_initialized) return;
	WriteRegister(0x02,0x9e);
	I2S_Init(SPI3, &(I2S_InitTypeDef){
		.I2S_Mode = I2S_Mode_MasterTx,
		.I2S_Standard = I2S_Standard_Phillips,
		.I2S_DataFormat = I2S_DataFormat_16b,
		.I2S_MCLKOutput = I2S_MCLKOutput_Enable,
		.I2S_AudioFreq = Frequency,
		.I2S_CPOL = I2S_CPOL_Low
	});
}

void AudioOff(void)
{
	if (!audio_initialized) return;
	WriteRegister(0x02,0x01);
	SPI3->I2SCFGR=0;
}

void SetAudioVolume(int volume)
{
	if (!audio_initialized) return;
	WriteRegister(0x20, (volume + 0x19) & 0xff);
	WriteRegister(0x21, (volume + 0x19) & 0xff);
}

_Bool IsAudioInitialized(void)
{
	return audio_initialized;
}

void OutputAudioSample(int16_t sample)
{
	if (!audio_initialized) return;
	while(!SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE));
	SPI_I2S_SendData(SPI3, sample);
}

void OutputAudioSampleWithoutBlocking(int16_t sample)
{
	if (!audio_initialized) return;
	SPI_I2S_SendData(SPI3, sample);
}

void PlayAudioWithCallback(AudioCallbackFunction *callback, void *context)
{
	if (!audio_initialized) return;
	StopAudioDMA();

	NVIC_Init(&(NVIC_InitTypeDef) {
		.NVIC_IRQChannel = DMA1_Stream7_IRQn,
		.NVIC_IRQChannelPreemptionPriority = 1,
		.NVIC_IRQChannelSubPriority = 0,
		.NVIC_IRQChannelCmd = ENABLE
	});
	DMA_ITConfig(DMA1_Stream7, DMA_IT_TC, ENABLE);
	SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Tx, ENABLE);

	CallbackFunction = callback;
	CallbackContext = context;
	BufferNumber = 0;

	if (CallbackFunction) {
		CallbackFunction(CallbackContext, SampleBuffer[BufferNumber]);
		ProvideAudioBuffer(SampleBuffer[BufferNumber], 256);
	}
}

void StopAudio(void)
{
	if (!audio_initialized) return;
	StopAudioDMA();

	SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Tx, DISABLE);
	NVIC_Init(&(NVIC_InitTypeDef) {
		.NVIC_IRQChannel = DMA1_Stream7_IRQn,
		.NVIC_IRQChannelCmd = DISABLE
	});

	CallbackFunction = NULL;
}

static void ProvideAudioBuffer(void *samples, int numsamples)
{
	while (!ProvideAudioBufferWithoutBlocking(samples, numsamples))
		__asm__ volatile ("wfi");
}

static bool ProvideAudioBufferWithoutBlocking(void *samples, int numsamples)
{
	if (NextBufferSamples) return false;

	NVIC_Init(&(NVIC_InitTypeDef) {
		.NVIC_IRQChannel = DMA1_Stream7_IRQn,
		.NVIC_IRQChannelCmd = DISABLE
	});

	NextBufferSamples = samples;
	NextBufferLength = numsamples;

	if (!DMARunning)
		StartAudioDMAAndRequestBuffers();

	NVIC_Init(&(NVIC_InitTypeDef) {
		.NVIC_IRQChannel = DMA1_Stream7_IRQn,
		.NVIC_IRQChannelPreemptionPriority = 1,
		.NVIC_IRQChannelSubPriority = 0,
		.NVIC_IRQChannelCmd = ENABLE
	});

	return true;
}

static void StartAudioDMAAndRequestBuffers()
{
	// Configure DMA stream.
	DMA_Init(DMA1_Stream7, &(DMA_InitTypeDef) {
		.DMA_Channel = 0,
		.DMA_PeripheralBaseAddr = (uint32_t)&SPI3->DR,
		.DMA_Memory0BaseAddr = (uint32_t)NextBufferSamples,
		.DMA_DIR = DMA_DIR_MemoryToPeripheral,
		.DMA_BufferSize = NextBufferLength,
		.DMA_PeripheralInc = DMA_PeripheralInc_Disable,
		.DMA_MemoryInc = DMA_MemoryInc_Enable,
		.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
		.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,
		.DMA_Mode = DMA_Mode_Normal,
		.DMA_Priority = DMA_Priority_Medium,
		.DMA_FIFOMode = DMA_FIFOMode_Disable,
		.DMA_MemoryBurst = DMA_MemoryBurst_Single,
		.DMA_PeripheralBurst = DMA_PeripheralBurst_Single
	});

	DMA_Cmd(DMA1_Stream7, ENABLE);

	// Update state
	NextBufferSamples = NULL;
	BufferNumber ^= 1;
	DMARunning = true;

	// Invoke callback if it exists to queue up another buffer
	if (CallbackFunction) {
		CallbackFunction(CallbackContext, SampleBuffer[BufferNumber]);
		ProvideAudioBuffer(SampleBuffer[BufferNumber], 256);
	}
}

static void StopAudioDMA()
{
	DMA_Cmd(DMA1_Stream7, DISABLE);
	while (DMA_GetCmdStatus(DMA1_Stream7)); // Wait for DMA stream to stop

	DMARunning = false;
}


void DMA1_Stream7_IRQHandler(void)
{
	DMA_ClearFlag(DMA1_Stream7, DMA_FLAG_TCIF7);

	if (NextBufferSamples)
		StartAudioDMAAndRequestBuffers();
	else
		DMARunning=false;
}

static void WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue)
{
	/* While the bus is busy */
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

	/* Start the config sequence */
	I2C_GenerateSTART(I2C1, ENABLE);

	/* Test on EV5 and clear it */
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/* Transmit the slave address and enable writing operation */
	I2C_Send7bitAddress(I2C1, 0x94, I2C_Direction_Transmitter);

	/* Test on EV6 and clear it */
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	/* Transmit the first address for write operation */
	I2C_SendData(I2C1, RegisterAddr);

	/* Test on EV8 and clear it */
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING));

	/* Prepare the register value to be sent */
	I2C_SendData(I2C1, RegisterValue);

	/* Wait till all data have been physically transferred on the bus */
	while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF));

	/* End the configuration sequence */
	I2C_GenerateSTOP(I2C1, ENABLE);
}

static uint32_t ReadRegister(uint8_t RegisterAddr)
{
	uint32_t result = 0;

	/*!< While the bus is busy */
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

	/* Start the config sequence */
	I2C_GenerateSTART(I2C1, ENABLE);

	/* Test on EV5 and clear it */
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/* Transmit the slave address and enable writing operation */
	I2C_Send7bitAddress(I2C1, 0x94, I2C_Direction_Transmitter);

	/* Test on EV6 and clear it */
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	/* Transmit the register address to be read */
	I2C_SendData(I2C1, RegisterAddr);

	/* Test on EV8 and clear it */
	while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF) == RESET);

	/*!< Send START condition a second time */
	I2C_GenerateSTART(I2C1, ENABLE);

	/*!< Test on EV5 and clear it (cleared by reading SR1 then writing to DR) */
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	/*!< Send Codec address for read */
	I2C_Send7bitAddress(I2C1, 0x94, I2C_Direction_Receiver);

	/* Wait on ADDR flag to be set (ADDR is still not cleared at this level */
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR) == RESET);

	/*!< Disable Acknowledgment */
	I2C_AcknowledgeConfig(I2C1, DISABLE);

	/* Clear ADDR register by reading SR1 then SR2 register (SR1 has already been read) */
	(void)I2C1->SR2;

	/*!< Send STOP Condition */
	I2C_GenerateSTOP(I2C1, ENABLE);

	/* Wait for the byte to be received */
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);

	/*!< Read the byte received from the Codec */
	result = I2C_ReceiveData(I2C1);

	/* Wait to make sure that STOP flag has been cleared */
	while(I2C1->CR1 & I2C_CR1_STOP);

	/*!< Re-Enable Acknowledgment to be ready for another reception */
	I2C_AcknowledgeConfig(I2C1, ENABLE);

	/* Clear AF flag for next communication */
	I2C_ClearFlag(I2C1, I2C_FLAG_AF);

	/* Return the byte read from Codec */
	return result;
}
