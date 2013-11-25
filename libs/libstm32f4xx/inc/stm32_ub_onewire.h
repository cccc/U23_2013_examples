//--------------------------------------------------------------
// File     : stm32_ub_onewire.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_ONEWIRE_H
#define __STM32F4_UB_ONEWIRE_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include <string.h>


//--------------------------------------------------------------
// Enable/Disable für die Umwandlung RomCode in einen String
// Das CoIDE-Modul "Retarget Printf" ist dazu notwendig
//--------------------------------------------------------------
#define  USE_ROMCODE2STR


#ifdef USE_ROMCODE2STR
  #include <stdio.h>
#endif




//--------------------------------------------------------------
// Datenpin der OneWire Slaves
//--------------------------------------------------------------
#define ONE_WIRE_PIN               GPIO_Pin_3
#define ONE_WIRE_PORT              GPIOD
#define ONE_WIRE_CLK               RCC_AHB1Periph_GPIOD


//--------------------------------------------------------------
// One-Wire ROM-Befehle
//--------------------------------------------------------------
#define  ONE_WIRE_READ_ROM_CMD     0x33   // auslesen einer Rom-ID
#define  ONE_WIRE_MATCH_ROM_CMD    0x55   // auswählen eines Slaves


//--------------------------------------------------------------
// 8Byte RomCode
// byte[0]     = Family-Code
// byte[1...6] = Serial-Nr
// byte[7]     = Checksum
//--------------------------------------------------------------
uint8_t one_wire_rom_code[8];


//--------------------------------------------------------------
// Timer
// F_TIM = 84 MHz / (prescaler+1) / (periode+1)
// F_TIM = 1MHz => 1us
//--------------------------------------------------------------
#define ONE_WIRE_PRESCALE   1
#define ONE_WIRE_PERIOD     41


//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void UB_OneWire_Init(void);
ErrorStatus UB_OneWire_ResetSequenz(void);
ErrorStatus UB_OneWire_ReadRomCode(void);
void UB_OneWire_WriteByte(uint8_t wert);
uint8_t UB_OneWire_ReadByte(void);
void UB_OneWire_WriteBitLo(void);
void UB_OneWire_WriteBitHi(void);
BitAction UB_OneWire_ReadBit(void);
ErrorStatus UB_OneWire_Str2RomCode(char *rom_code);
#ifdef USE_ROMCODE2STR
void UB_OneWire_RomCode2Str(char *rom_code);
#endif


//--------------------------------------------------------------
#endif // __STM32F4_UB_ONEWIRE_H
