#include <System.h>
#include <stm32f4xx.h>
#include <stdio.h>

#define SLAVE_ADDRESS 0x68

#define SECS  0
#define MINS  1
#define HRS   2
#define DAY   3
#define DATE  4
#define MONTH 5
#define YEAR  6

// We have to initilize the clock. Let's take with the beginning time of the first U23-2013 meeting.
#define I_HRS   0x11 // 11:
#define I_MINS  0x00 // 00:
#define I_SECS  0x00 // 00,
#define I_DAY   0x06 // Saturday
#define I_DATE  0x19 // 19
#define I_MONTH 0x10 // October,
#define I_YEAR  0x13 // (20)13.
// TODO: write some code (together with some scripts?) to set the real time clock to current time of the computer

void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction);
void I2C_restart(I2C_TypeDef* I2Cx, uint8_t addr, uint8_t direction);
void I2C_write(I2C_TypeDef* I2Cx, uint8_t data);
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx);
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx);
void I2C_stop(I2C_TypeDef* I2Cx);

// We will need to convert binary-coded decimal numbers into normal decimals.
uint8_t bcd_to_decimal(uint8_t n);

/* This code, which is based on 09_i2c.c, talks with a DS1307 real-time clock via I2C, and (re)sets the clock before doing so.
 * You don't need extra pullup resistors since the clock already has them, you can just connect the appropriate 4 pins (ground, 5V power input, SDA and SCL) to begin.
 * (The pins on the side with seven pins are known to work.)
 */

int main()
{
	// Do some basic initialization tasks
	InitializeSystem();

	EnableDebugOutput(DEBUG_ITM);

	// Enable clock for GPIOA and GPIOC peripheral
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	// Set Pin modes for PA8 (SCL)
	GPIO_Init(GPIOA, &(GPIO_InitTypeDef){
		.GPIO_Speed = GPIO_Speed_2MHz,
		.GPIO_Mode = GPIO_Mode_AF,		//Not OUT, but AF = Alternate Function
		.GPIO_OType = GPIO_OType_OD,
		.GPIO_PuPd = GPIO_PuPd_NOPULL,	//External pullups, so none here
		.GPIO_Pin = GPIO_Pin_8
	});

	// Set Pin modes for PC9 (SDA)
	GPIO_Init(GPIOC, &(GPIO_InitTypeDef){
		.GPIO_Speed = GPIO_Speed_2MHz,
		.GPIO_Mode = GPIO_Mode_AF,		//Not OUT, but AF = Alternate Function
		.GPIO_OType = GPIO_OType_OD,
		.GPIO_PuPd = GPIO_PuPd_NOPULL,	//External pullups, so none here
		.GPIO_Pin = GPIO_Pin_9
	});

	//Attach alternate pin functions
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_I2C3);	//SCL
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_I2C3);	//SDA

	//Enable I2C3 clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C3, ENABLE);

	//Configure I2C3
	//NOTE: Excerpt of the source code: "To use the I2C at 400 KHz (in fast mode), the PCLK1 frequency (I2C peripheral input clock) must be a multiple of 10 MHz."
	//PCLK1 is 168MHz in our case and is divided by 4 to get PCLK1 which makes it 42 which is not a multiple of 10. We can't use 400kHz here.
	I2C_Init(I2C3, &(I2C_InitTypeDef){
		.I2C_ClockSpeed = 100000,			//see note above
		.I2C_Mode = I2C_Mode_I2C,			//we want raw I2C, no SMBUS or other stuff
		.I2C_DutyCycle = I2C_DutyCycle_2,	//only relevant for fast mode
		.I2C_OwnAddress1 = 0xEE,			//only relevant for slave mode
		.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit,	//7-bit addresses, only relevant for slave mode
		.I2C_Ack = I2C_Ack_Disable			//wether or not to acknowledge automatically
	});

	//Turn that thing on...
	I2C_Cmd(I2C3, ENABLE);

	// reset/initialize the clock
	I2C_start(I2C3, SLAVE_ADDRESS<<1, I2C_Direction_Transmitter);
	I2C_write(I2C3, 0x00); // set pointer to first (seconds) register
	I2C_write(I2C3, I_SECS); // reset the clock halt bit while setting the seconds
	I2C_write(I2C3, I_MINS);
	I2C_write(I2C3, I_HRS); // 24h mode is implied since the 6th bit is 0
	I2C_write(I2C3, I_DAY);
	I2C_write(I2C3, I_DATE);
	I2C_write(I2C3, I_MONTH);
	I2C_write(I2C3, I_YEAR);
	I2C_stop(I2C3);

	// get ready to read and print the time
	int8_t time[7];
	char days[7][3] = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};
	char months[12][4] ={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	while(1)
	{
		I2C_start(I2C3, SLAVE_ADDRESS<<1, I2C_Direction_Transmitter);
		I2C_write(I2C3, 0x00); // set pointer to seconds register

		//do a repeated start, we want to read now
		I2C_restart(I2C3, SLAVE_ADDRESS<<1, I2C_Direction_Receiver);

		// read the time and convert the numbers to decimal where needed
		time[SECS] = bcd_to_decimal(I2C_read_ack(I2C3));
		time[MINS] = bcd_to_decimal(I2C_read_ack(I2C3));
		time[HRS] = bcd_to_decimal(I2C_read_ack(I2C3));
		time[DAY] = I2C_read_ack(I2C3);
		time[DATE] = bcd_to_decimal(I2C_read_ack(I2C3));
		time[MONTH] = bcd_to_decimal(I2C_read_ack(I2C3));
		time[YEAR] = bcd_to_decimal(I2C_read_nack(I2C3));

		iprintf("\n%ih %im %is\n%s %s %i, %i\n", time[HRS], time[MINS], time[SECS], days[time[DAY]-1], months[time[MONTH]-1], time[DATE], time[YEAR]);

		Delay(100);
	}
}


void I2C_start(I2C_TypeDef* I2Cx, uint8_t address, uint8_t direction)
{
	// wait until I2C1 is not busy anymore
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));

	// Send I2C1 START condition 
	I2C_GenerateSTART(I2Cx, ENABLE);
	
	// wait for I2C1 EV5 --> Slave has acknowledged start condition
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

	// Send slave Address for write 
	I2C_Send7bitAddress(I2Cx, address, direction);
	
	/* wait for I2C1 EV6, check if 
	 * either Slave has acknowledged Master transmitter or
	 * Master receiver mode, depending on the transmission
	 * direction
	 */ 
	if(direction == I2C_Direction_Transmitter)
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	else if(direction == I2C_Direction_Receiver)
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
}

void I2C_restart(I2C_TypeDef* I2Cx, uint8_t addr, uint8_t direction)
{
	// Send I2C1 START condition
	I2C_GenerateSTART(I2Cx, ENABLE);

	// wait for I2C1 EV5 --> Slave has acknowledged start condition
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

	// Send slave Address for read or write
	I2C_Send7bitAddress(I2Cx, addr, direction);

	//------------------------------------------------------------------------
	// wait for I2C1 EV6, check if Slave has acknowledged Master transmitter
	// or Master receiver mode, depending on the transmission direction
	//------------------------------------------------------------------------
	if(direction == I2C_Direction_Transmitter)
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	else if(direction == I2C_Direction_Receiver)
		while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
}

void I2C_write(I2C_TypeDef* I2Cx, uint8_t data)
{
	I2C_SendData(I2Cx, data);
	// wait for I2C1 EV8_2 --> byte has been transmitted
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

uint8_t I2C_read_ack(I2C_TypeDef* I2Cx)
{
	// enable acknowledge of recieved data
	I2C_AcknowledgeConfig(I2Cx, ENABLE);
	// wait until one byte has been received
	while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

uint8_t I2C_read_nack(I2C_TypeDef* I2Cx)
{
	// disabe acknowledge of received data
	// nack also generates stop condition after last byte received
	// see reference manual for more info
	I2C_AcknowledgeConfig(I2Cx, DISABLE);
	I2C_GenerateSTOP(I2Cx, ENABLE);
	// wait until one byte has been received
	while( !I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) );
	// read data from I2C data register and return data byte
	uint8_t data = I2C_ReceiveData(I2Cx);
	return data;
}

void I2C_stop(I2C_TypeDef* I2Cx)
{
	// Send I2C1 STOP Condition 
	I2C_GenerateSTOP(I2Cx, ENABLE);
}

uint8_t bcd_to_decimal(uint8_t n)
{
	// first four binary digits are for the first decimal digit, and the last four for the next one
	return 10*(n >> 4) + (n & 0x0f);
}
