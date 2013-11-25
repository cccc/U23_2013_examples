//--------------------------------------------------------------
// File     : stm32_ub_ds18xx.c
// Datum    : 29.09.2013
// Version  : 1.2
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : STM32_UB_ONEWIRE, (Retarget Printf)
// Funktion : Für OneWire Temp-Sensoren
//
// Sensor-Typen : DS1822, DS18B20, DS1820, DS18S20
// Hinweise     : externer 4k7 PullUp notwendig
//                Spannungsversorgung nicht per Datenleitung !!
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_ds18xx.h"



//--------------------------------------------------------------
// globale Variabeln
//--------------------------------------------------------------
uint8_t ds_data_read[9]; // zum speichern der Sensor-Daten



//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
float P_DS18XX_readTemp(char *rom_code, uint8_t typ);
float P_DS1822B20_calcTemp(void);
float P_DS1820S20_calcTemp(void);



//--------------------------------------------------------------
// Init-Funktion
//
// Return_wert :
//  -> ERROR   , kein Slave wurde gefunden
//  -> SUCCESS , mindestens ein Slave hängt am BUS
//--------------------------------------------------------------
ErrorStatus UB_DS18XX_Init(void)
{
  ErrorStatus ret_wert=ERROR;

  // init der OneWire-Lib
  UB_OneWire_Init();

  // check ob ein Slave am Bus vorhanden ist
  ret_wert=UB_OneWire_ResetSequenz();

  return(ret_wert);
}


//--------------------------------------------------------------
// auslesen der Temperatur von einem DS1822 oder DS18B20
// rom_code : muss als String mit 16 Hex-Ziffern übergeben werden
//    z.B. "220E6D33000000BB"
//    Zahl-1 "22"  = one_wire_rom_code[0]    = Family-Code
//    Zahl-2 bis 7 = one_wire_rom_code[1..6] = Serial-Nr
//    Zahl-8 "BB"  = one_wire_rom_code[7]    = Checksum
//
// return_wert : Temperatur in Grad-Celsius [+125.0 bis -55.0]
// bei einem Fehler wird "DS18XX_ERR_CODE" zurueckgegeben
//--------------------------------------------------------------
float UB_DS1822B20_ReadTemp(char *rom_code)
{
  float ret_wert=0.0;

  // Typ = DS1822, DS18B20
  ret_wert=P_DS18XX_readTemp(rom_code, 0);

  return(ret_wert);
}


//--------------------------------------------------------------
// auslesen der Temperatur von einem DS1820 oder DS18S20
// rom_code : muss als String mit 16 Hex-Ziffern übergeben werden
//    z.B. "220E6D33000000BB"
//    Zahl-1 "22"  = one_wire_rom_code[0]    = Family-Code
//    Zahl-2 bis 7 = one_wire_rom_code[1..6] = Serial-Nr
//    Zahl-8 "BB"  = one_wire_rom_code[7]    = Checksum
//
// return_wert : Temperatur in Grad-Celsius [+125.0 bis -55.0]
// bei einem Fehler wird "DS18XX_ERR_CODE" zurueckgegeben
//--------------------------------------------------------------
float UB_DS1820S20_ReadTemp(char *rom_code)
{
  float ret_wert=0.0;

  // Typ = DS1820, DS18S20
  ret_wert=P_DS18XX_readTemp(rom_code, 1);

  return(ret_wert);
}


//--------------------------------------------------------------
// stellt die Aufloesung vom DS1822 oder DS18B20 ein
// rom_code : muss als String mit 16 Hex-Ziffern übergeben werden
// bit : [9...12]
//   12bit => Aufloesung = 0,0625 Grad (ConvTime = ca. 750 ms)
//   11bit => Aufloesung = 0,125 Grad  (ConvTime = ca. 375 ms)
//   10bit => Aufloesung = 0,25 Grad   (ConvTime = ca. 187 ms)
//    9bit => Aufloesung = 0,5 Grad    (ConvTime = ca.  93 ms)
//
// Return_wert :
//  -> ERROR   , Fehler
//  -> SUCCESS , alles Ok
//--------------------------------------------------------------
ErrorStatus UB_DS1822B20_SetResolution(char *rom_code, uint8_t bit)
{
  ErrorStatus ret_wert=ERROR;
  ErrorStatus check;
  uint8_t conf_reg=0x00;
  uint32_t n;

  if(bit==9) conf_reg=0x00;
  if(bit==10) conf_reg=0x20;
  if(bit==11) conf_reg=0x40;
  if(bit==12) conf_reg=0x60;

  // RomCode umwandeln
  check=UB_OneWire_Str2RomCode(rom_code);
  if(check==SUCCESS) {
    // wenn RomCode ok
    if(UB_OneWire_ResetSequenz()==SUCCESS) {
      // befehl senden (auswählen eines Slaves)
      UB_OneWire_WriteByte(ONE_WIRE_MATCH_ROM_CMD);
      // Rom-Code senden
      for(n=0;n<8;n++) {
        UB_OneWire_WriteByte(one_wire_rom_code[n]);
      }
      // befehl senden (schreiben vom Config-Reg)
      UB_OneWire_WriteByte(ONE_WIRE_WR_SCRATCH_FKT_CMD);
      // 3 Bytes zum Slave senden
      UB_OneWire_WriteByte(0x7F); // Alarm High Register
      UB_OneWire_WriteByte(0x80); // Alarm Low Register
      UB_OneWire_WriteByte(conf_reg); // Config Byte
      // reset seq
      UB_OneWire_ResetSequenz();
      ret_wert=SUCCESS;
    }
  }  

  return(ret_wert);   
}


//--------------------------------------------------------------
// speichert die Aufloesung vom DS1822 oder DS18B20 im EEprom
// Vorsicht : EEprom hat nur 50.000 schreibzyklen
// rom_code : muss als String mit 16 Hex-Ziffern übergeben werden
// bit : [9...12]
//   12bit => Aufloesung = 0,0625 Grad (ConvTime = ca. 750 ms)
//   11bit => Aufloesung = 0,125 Grad  (ConvTime = ca. 375 ms)
//   10bit => Aufloesung = 0,25 Grad   (ConvTime = ca. 187 ms)
//    9bit => Aufloesung = 0,5 Grad    (ConvTime = ca.  93 ms)
//
// Return_wert :
//  -> ERROR   , Fehler
//  -> SUCCESS , alles Ok
//--------------------------------------------------------------
ErrorStatus UB_DS1822B20_WriteResolution(char *rom_code, uint8_t bit)
{
  ErrorStatus ret_wert=ERROR;
  BitAction wait_bit;
  uint32_t timeout=0,n;

  ret_wert=UB_DS1822B20_SetResolution(rom_code, bit);
  if(ret_wert==SUCCESS) {
    if(UB_OneWire_ResetSequenz()==SUCCESS) {
      // befehl senden (auswählen eines Slaves)
      UB_OneWire_WriteByte(ONE_WIRE_MATCH_ROM_CMD);
      // Rom-Code senden
      for(n=0;n<8;n++) {
        UB_OneWire_WriteByte(one_wire_rom_code[n]);
      }
      // befehl senden (kopieren vom Config-Reg ins EEprom)
      UB_OneWire_WriteByte(ONE_WIRE_COPY_SCRATCH_FKT_CMD);
      // warten bis schreiben abgeschlossen (oder Timeout)
      timeout=0;
      do {
        wait_bit=UB_OneWire_ReadBit();
        timeout++;
      }while((wait_bit==Bit_RESET) && (timeout<DS18XX_TIMEOUT));

      if(timeout<DS18XX_TIMEOUT) {
        // wenn schreiben fertig
        // reset seq
        UB_OneWire_ResetSequenz();
        ret_wert=SUCCESS;
      }
      else {
        // bei Timeout -> reset seq
        UB_OneWire_ResetSequenz();
      }
    }
  }

  return(ret_wert);
}


//--------------------------------------------
// wandelt eine float Zahl in einen String
// mit Vorzeichen und 3 Nachkommastellen
//--------------------------------------------
#ifdef USE_TEMP2STR
void UB_DS18XX_TempToStr(float wert, char *ptr)
{
  int16_t vorkomma;
  uint16_t nachkomma;
  float rest;

  vorkomma=(int16_t)(wert);
  if(wert>=0) {
    rest = wert-(float)(vorkomma);
  }
  else {
    rest = (float)(vorkomma)-wert;
  }
  nachkomma = (uint16_t)(rest*(float)(1000)+0.5);

  sprintf(ptr,"%d.%03d",vorkomma,nachkomma);
}
#endif


//--------------------------------------------------------------
// interne Funktion
// zum auslesen der Temperatur von einem Sensor
// return_wert : Temperatur in Grad-Celsius [+125.0 bis -55.0]
// bei einem Fehler wird "DS18XX_ERR_CODE" zurueckgegeben
// typ : 0 = DS1822, DS18B20
//       1 = DS1820, DS18S20
//--------------------------------------------------------------
float P_DS18XX_readTemp(char *rom_code, uint8_t typ)
{
  float ret_wert=DS18XX_ERR_CODE;
  ErrorStatus check;
  BitAction wait_bit;
  uint32_t timeout=0,n;

  // RomCode umwandeln
  check=UB_OneWire_Str2RomCode(rom_code);
  if(check==SUCCESS) {
    // wenn RomCode ok
    if(UB_OneWire_ResetSequenz()==SUCCESS) {
      // befehl senden (auswählen eines Slaves)
      UB_OneWire_WriteByte(ONE_WIRE_MATCH_ROM_CMD);
      // Rom-Code senden
      for(n=0;n<8;n++) {
        UB_OneWire_WriteByte(one_wire_rom_code[n]);
      }
      // befehl senden (starten der Temp-Messung)
      UB_OneWire_WriteByte(ONE_WIRE_CONVERT_FKT_CMD);
      // warten bis wandlung abgeschlossen (oder Timeout)
      timeout=0;
      do {
        wait_bit=UB_OneWire_ReadBit();
        timeout++;
      }while((wait_bit==Bit_RESET) && (timeout<DS18XX_TIMEOUT));

      if(timeout<DS18XX_TIMEOUT) {
        // wenn wandlung fertig
        if(UB_OneWire_ResetSequenz()==SUCCESS) {
          // befehl senden (auswählen eines Slaves)
          UB_OneWire_WriteByte(ONE_WIRE_MATCH_ROM_CMD);
          // Rom-Code senden
          for(n=0;n<8;n++) {
            UB_OneWire_WriteByte(one_wire_rom_code[n]);
          }
          // befehl senden (auslesen der Temperatur)
          UB_OneWire_WriteByte(ONE_WIRE_RD_SCRATCH_FKT_CMD);
          // 9 Bytes vom Slave auslesen
          for(n=0;n<9;n++) {
            ds_data_read[n]=UB_OneWire_ReadByte();
          }
          // reset seq
          UB_OneWire_ResetSequenz();
          // temperatur ausrechnen
          if(typ==0) ret_wert=P_DS1822B20_calcTemp();
          if(typ==1) ret_wert=P_DS1820S20_calcTemp();        
        }
      }
      else {
        // bei Timeout -> reset seq
        UB_OneWire_ResetSequenz();
      }
    }
  }
  
  return(ret_wert);
}


//--------------------------------------------
// interne Funktion
// ausrechnen der temperatur in Grad-Celsius
// je nach Aufloesung
//
// fuer DS1822 oder DS18B20
//
// Aufloesung : [9bit...12bit]
//   12bit => Aufloesung = 0,0625 Grad
//   11bit => Aufloesung = 0,125 Grad
//   10bit => Aufloesung = 0,25 Grad
//    9bit => Aufloesung = 0,5 Grad
//
// ret_wert :
//    +125,0 Grad bis -55,0 Grad
//--------------------------------------------
float P_DS1822B20_calcTemp(void)
{
  float ret_wert=0.0;
  uint8_t data_lo, data_hi, conf_reg;
  uint16_t raw_temp;

  data_lo=ds_data_read[0];  // temp lo-byte
  data_hi=ds_data_read[1];  // temp hi-byte
  conf_reg=ds_data_read[4]; // config register

  if((conf_reg&0x60)==0x60) {
    // 12bit Aufloesung
    raw_temp=(data_hi&0x07)<<8;
    raw_temp|=data_lo;
    if((data_hi&0x80)==0) {
      ret_wert=(float)(raw_temp)*(0.0625);
    }
    else {
      raw_temp^=0x07FF;
      raw_temp++;
      ret_wert=(float)(raw_temp)*(-0.0625);
    }
  }
  else if((conf_reg&0x60)==0x40) {
    // 11bit Aufloesung
    raw_temp=(data_hi&0x07)<<7;
    raw_temp|=(data_lo>>1);
    if((data_hi&0x80)==0) {
      ret_wert=(float)(raw_temp)*(0.125);
    }
    else {
      raw_temp^=0x03FF;
      raw_temp++;
      ret_wert=(float)(raw_temp)*(-0.125);
    }
  }
  else if((conf_reg&0x60)==0x20) {
    // 10bit Aufloesung
    raw_temp=(data_hi&0x07)<<6;
    raw_temp|=(data_lo>>2);
    if((data_hi&0x80)==0) {
      ret_wert=(float)(raw_temp)*(0.25);
    }
    else {
      raw_temp^=0x01FF;
      raw_temp++;
      ret_wert=(float)(raw_temp)*(-0.25);
    }
  }
  else if((conf_reg&0x60)==0x00) {
    // 9bit Aufloesung
    raw_temp=(data_hi&0x07)<<5;
    raw_temp|=(data_lo>>3);
    if((data_hi&0x80)==0) {
      ret_wert=(float)(raw_temp)*(0.5);
    }
    else {
      raw_temp^=0x00FF;
      raw_temp++;
      ret_wert=(float)(raw_temp)*(-0.5);
    }
  }

  return(ret_wert);
}


//--------------------------------------------
// interne Funktion
// ausrechnen der temperatur in Grad-Celsius
//
// fuer DS1820 oder DS18S20
//
// Aufloesung : [9bit]
//    9bit => Aufloesung = 0,5 Grad
//
// ret_wert :
//    +125,0 Grad bis -55,0 Grad
//--------------------------------------------
float P_DS1820S20_calcTemp(void)
{
  float ret_wert=0.0;
  uint8_t data_lo, data_hi;
  uint8_t raw_temp;

  data_lo=ds_data_read[0];  // temp lo-byte
  data_hi=ds_data_read[1];  // temp hi-byte

  // 9bit Aufloesung
  raw_temp=data_lo;
  if((data_hi&0x80)==0) {
    ret_wert=(float)(raw_temp)*(0.5);
  }
  else {
    raw_temp^=0xFF;
    raw_temp++;
    ret_wert=(float)(raw_temp)*(-0.5);
  }

  return(ret_wert);
}

