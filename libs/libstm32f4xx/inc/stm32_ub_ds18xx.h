//--------------------------------------------------------------
// File     : stm32_ub_ds18xx.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_DS18XX_H
#define __STM32F4_UB_DS18XX_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32_ub_onewire.h"



//--------------------------------------------------------------
// Enable/Disable für die Umwandlung der Temperatur in einen String
// Das CoIDE-Modul "Retarget Printf" ist dazu notwendig
//--------------------------------------------------------------
#define  USE_TEMP2STR


#ifdef USE_TEMP2STR
  #include <stdio.h>
#endif



//--------------------------------------------------------------
// One-Wire Funktions-Befehle
//--------------------------------------------------------------
#define  ONE_WIRE_CONVERT_FKT_CMD       0x44  // starten der Temp-Messung
#define  ONE_WIRE_RD_SCRATCH_FKT_CMD    0xBE  // auslesen der Temperatur
#define  ONE_WIRE_WR_SCRATCH_FKT_CMD    0x4E  // schreiben vom Config-Reg
#define  ONE_WIRE_COPY_SCRATCH_FKT_CMD  0x48  // kopieren vom Config-Reg ins EEprom


//--------------------------------------------------------------
// defines
//--------------------------------------------------------------
#define  DS18XX_TIMEOUT    50000
#define  DS18XX_ERR_CODE   -1000  // bei Fehler wird diese Temp gemeldet



//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
ErrorStatus UB_DS18XX_Init(void);
float UB_DS1822B20_ReadTemp(char *rom_code);
float UB_DS1820S20_ReadTemp(char *rom_code);
ErrorStatus UB_DS1822B20_SetResolution(char *rom_code, uint8_t bit);
ErrorStatus UB_DS1822B20_WriteResolution(char *rom_code, uint8_t bit);
#ifdef USE_TEMP2STR
void UB_DS18XX_TempToStr(float wert, char *ptr);
#endif


//--------------------------------------------------------------
#endif // __STM32F4_UB_DS18XX_H
