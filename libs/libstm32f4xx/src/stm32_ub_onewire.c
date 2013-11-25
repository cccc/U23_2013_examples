//--------------------------------------------------------------
// File     : stm32_ub_onewire.c
// Datum    : 29.09.2013
// Version  : 1.1
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, TIM, MISC, (Retarget Printf)
// Funktion : One-Wire LoLevel Library (1-Wire)
// 
// Hinweis  : benutzt wird Timer-7
// Data-Pin : PD3 (ein externer PullUp von 4k7 ist notwendig)
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_onewire.h"
#include <stdio.h>

//--------------------------------------------------------------
// globale Variabeln
//--------------------------------------------------------------
uint32_t ow_akt_delay;      // fuer den Timer
uint8_t ow_ascii_err_flag;  // Fehlerflag



//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
uint8_t P_OW_STR2HEX(char *ptr, uint8_t start);
uint8_t P_OW_ASC2HEX(uint8_t ascii);
void P_OW_InitGPIO(void);
void P_OW_InitNVIC(void);
void P_OW_InitTIM(void);
void P_OW_Delay_us(uint32_t delay);
uint32_t P_OW_Mess_Lo_us(uint32_t delay);
void P_OW_DataLo(void);
void P_OW_DataHi(void);
BitAction P_OW_DataRead(void);






//--------------------------------------------------------------
// init der OneWire (1-Wire) Library
//--------------------------------------------------------------
void UB_OneWire_Init(void)
{
  static uint32_t init_ok=0;

  // initialisierung darf nur einmal gemacht werden
  if(init_ok!=0) {
    return;
  } 

  // GPIO init
  P_OW_InitGPIO();

  // Timer init
  P_OW_InitTIM();

  // NVIC init
  P_OW_InitNVIC();

  // Rom-Code loeschen
  one_wire_rom_code[0]=0x00;
  one_wire_rom_code[1]=0x00;
  one_wire_rom_code[2]=0x00;
  one_wire_rom_code[3]=0x00;
  one_wire_rom_code[4]=0x00;
  one_wire_rom_code[5]=0x00;
  one_wire_rom_code[6]=0x00;
  one_wire_rom_code[7]=0x00;

  // init Mode speichern
  init_ok=1;
}


//--------------------------------------------------------------
// Reset-Sequenz fuer alle Slaves am OneWire-Bus
// und check ob mindestens ein Slave angeschlossen ist
//
// Return_wert :
//  -> ERROR   , kein Slave wurde gefunden
//  -> SUCCESS , mindestens ein Slave hängt am BUS
//--------------------------------------------------------------
ErrorStatus UB_OneWire_ResetSequenz(void)
{
  ErrorStatus ret_wert=ERROR;
  uint32_t lo_time=0;
  BitAction hi_pegel;

  // Initialisierung-Sequenz
  P_OW_Delay_us(100);
  P_OW_DataLo();
  P_OW_Delay_us(560); //  480<T<640
  P_OW_DataHi();
  P_OW_Delay_us(20);  // 15<T<60
  lo_time=P_OW_Mess_Lo_us(300); // messen vom presence puls 60<T<240
  P_OW_Delay_us(50);
  hi_pegel=P_OW_DataRead(); // messen ob am ende wieder hi
  P_OW_Delay_us(120);

  // Auswertung ob ok
  if((lo_time>30) && (hi_pegel==Bit_SET)) {
    // der presence Puls vom Slave muss > 30 us sein
    ret_wert=SUCCESS;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// auslesen des 8Byte Rom-Codes von einem Slave
// Vorsicht !! es darf nur ein Slave am Bus hängen !!
//
// der gelesene ROM-Code steht danach in "one_wire_rom_code[0..7]"
//
// Return_wert :
//  -> ERROR   , wenn ROM-Code nicht ausgelesen wurde
//  -> SUCCESS , wenn ROM-Code ausgelesen wurde
//--------------------------------------------------------------
ErrorStatus UB_OneWire_ReadRomCode(void)
{
  ErrorStatus ret_wert=ERROR;
  uint32_t n;

  if(UB_OneWire_ResetSequenz()==SUCCESS) {
    // befehl senden (auslesen einer Rom-ID)
    UB_OneWire_WriteByte(ONE_WIRE_READ_ROM_CMD);
    // Rom-Code auslesen
    for(n=0;n<8;n++) {
      one_wire_rom_code[n]=UB_OneWire_ReadByte();
      //printf("%x", one_wire_rom_code[n]);
    }
    // reset seq
    UB_OneWire_ResetSequenz();
    ret_wert=SUCCESS;
  }
  else {
    // Rom-Code loeschen
    for(n=0;n<8;n++) {
      one_wire_rom_code[n]=0x00;
    }
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// sendet ein Byte per OneWire
//--------------------------------------------------------------
void UB_OneWire_WriteByte(uint8_t wert)
{
  uint8_t bit_maske=0x01,n;

  // 8bit per OneWire senden
  for(n=0;n<8;n++) {
    if((wert&bit_maske)==0) {
      UB_OneWire_WriteBitLo();
    }
    else {
      UB_OneWire_WriteBitHi();
    }
    bit_maske=bit_maske<<1;
  }
}


//--------------------------------------------------------------
// auslesen von einem Byte per OneWire
//--------------------------------------------------------------
uint8_t UB_OneWire_ReadByte(void)
{
  uint8_t ret_wert=0;
  uint8_t bit_maske=0x01,n;

  // 8bit per OneWire lesen
  for(n=0;n<8;n++) {
    if(UB_OneWire_ReadBit()==Bit_SET) {
      ret_wert|=bit_maske;
    }
    bit_maske=bit_maske<<1;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// sendet ein Lo-Bit per OneWire
//--------------------------------------------------------------
void UB_OneWire_WriteBitLo(void)
{
  P_OW_DataLo();
  P_OW_Delay_us(80);
  P_OW_DataHi();
  P_OW_Delay_us(5); // recovery
}


//--------------------------------------------------------------
// sendet ein HI-Bit per OneWire
//--------------------------------------------------------------
void UB_OneWire_WriteBitHi(void)
{
  P_OW_DataLo();
  P_OW_Delay_us(5);
  P_OW_DataHi();
  P_OW_Delay_us(80);
}


//--------------------------------------------------------------
// auslesen von einem Bit per OneWire
//  -> bei Lo-Pegel = Bit_RESET
//  -> bei Hi-Pegel = Bit_SET
//--------------------------------------------------------------
BitAction UB_OneWire_ReadBit(void)
{
  BitAction ret_wert;

  P_OW_DataLo();
  P_OW_Delay_us(5);
  P_OW_DataHi();
  P_OW_Delay_us(10);
  ret_wert=P_OW_DataRead();
  P_OW_Delay_us(50);

  return(ret_wert);
}


//--------------------------------------------------------------
// Hilfsfunktion :
// kopiert einen String mit 16 Hex-Ziffern
// in den 8Byte RomCode-Puffer
//
// rom_code : muss als String mit 16 Hex-Ziffern übergeben werden
//    z.B. "220E6D33000000BB"
//    Zahl-1 "22"  = one_wire_rom_code[0]    = Family-Code
//    Zahl-2 bis 7 = one_wire_rom_code[1..6] = Serial-Nr
//    Zahl-8 "BB"  = one_wire_rom_code[7]    = Checksum
//
// Return_wert :
//  -> ERROR   , der String hat nicht das passende Format
//  -> SUCCESS , der RomCode wurde kopiert
//--------------------------------------------------------------
ErrorStatus UB_OneWire_Str2RomCode(char *rom_code)
{
  ErrorStatus ret_wert=ERROR;
  uint8_t len;

  len=strlen(rom_code);
  if(len==16) {
    // Fehler Flag loeschen
    ow_ascii_err_flag=0;
    // 8Bytes umwandeln
    one_wire_rom_code[0]=P_OW_STR2HEX(rom_code,0);
    one_wire_rom_code[1]=P_OW_STR2HEX(rom_code,2);
    one_wire_rom_code[2]=P_OW_STR2HEX(rom_code,4);
    one_wire_rom_code[3]=P_OW_STR2HEX(rom_code,6);
    one_wire_rom_code[4]=P_OW_STR2HEX(rom_code,8);
    one_wire_rom_code[5]=P_OW_STR2HEX(rom_code,10);
    one_wire_rom_code[6]=P_OW_STR2HEX(rom_code,12);
    one_wire_rom_code[7]=P_OW_STR2HEX(rom_code,14);
    // test ob fehler aufgetreten ist
    if(ow_ascii_err_flag==0) ret_wert=SUCCESS;
  }
  else {
    ret_wert=ERROR; // laenge falsch
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// Hilfsfunktion :
// kopiert den 8Byte RomCode
// in einen String mit 16 Hex-Ziffern
//
// diese Funktion muss direkt nach "UB_OneWire_ReadRomCode"
// benutzt werden, um den RomCode als String darzustellen
//--------------------------------------------------------------
#ifdef USE_ROMCODE2STR
void UB_OneWire_RomCode2Str(char *rom_code)
{
  sprintf(rom_code,"%02X%02X%02X%02X%02X%02X%02X%02X",
    one_wire_rom_code[0],one_wire_rom_code[1],one_wire_rom_code[2],one_wire_rom_code[3],
    one_wire_rom_code[4],one_wire_rom_code[5],one_wire_rom_code[6],one_wire_rom_code[7]);
}
#endif


//--------------------------------------------------------------
// interne Funktion
// wandelt einen Teilstring mit 2 Hex-Ziffern in eine Zahl um
//  z.B. "1f" oder "1F" => 31
//--------------------------------------------------------------
uint8_t P_OW_STR2HEX(char *ptr, uint8_t start)
{
  uint8_t ret_wert=0;
  char *p;
  uint8_t wert;

  // pointer setzen
  p=ptr+start;

  // erste Ziffer
  wert=P_OW_ASC2HEX(*p);
  ret_wert=wert*16;
  // zweite Ziffer
  p++;
  wert=P_OW_ASC2HEX(*p);
  ret_wert+=wert;

  return(ret_wert);
}


//--------------------------------------------------------------
// interne Funktion
// wandelt ein Ascii-Zeichen in eine Zahl um
//  z.B. 'a' oder 'A' => 10
//  bei einem ungueltigen Zeichen wird ein Fehler Flag gesetzt
//--------------------------------------------------------------
uint8_t P_OW_ASC2HEX(uint8_t ascii)
{
  uint8_t ret_wert=0;

  if((ascii>=48) && (ascii<=57)) {
    ret_wert=ascii-48;
  }
  else if((ascii>=65) && (ascii<=70)) {
    ret_wert=ascii-55;
  }
  else if((ascii>=97) && (ascii<=102)) {
    ret_wert=ascii-87;
  }
  else {
    ow_ascii_err_flag=1;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// interne Funktion
// init vom GPIO-Pin
//--------------------------------------------------------------
void P_OW_InitGPIO(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  // Clock enable
  RCC_AHB1PeriphClockCmd(ONE_WIRE_CLK, ENABLE);

  // Config als OpenDrain-Ausgang
  GPIO_InitStructure.GPIO_Pin = ONE_WIRE_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(ONE_WIRE_PORT, &GPIO_InitStructure);

  // Hi-Pegel ausgeben
  P_OW_DataHi();
}


//--------------------------------------------------------------
// interne Funktion
// init vom NVIC
//--------------------------------------------------------------
void P_OW_InitNVIC(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  // Update Interrupt enable
  TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE);

  // NVIC konfig
  NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


//--------------------------------------------------------------
// interne Funktion
// init vom Timer
//--------------------------------------------------------------
void P_OW_InitTIM(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  // Clock enable
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

  // Timer disable
  TIM_Cmd(TIM7, DISABLE);

  // Timer auf 1us einstellen
  TIM_TimeBaseStructure.TIM_Period=ONE_WIRE_PERIOD;
  TIM_TimeBaseStructure.TIM_Prescaler=ONE_WIRE_PRESCALE;
  TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

  TIM_ARRPreloadConfig(TIM7, ENABLE);
}


//------------------------------------------------------------
// interne Funktion
// Delay-Funktion (us)
//------------------------------------------------------------
void P_OW_Delay_us(uint32_t delay)
{
  // Timer auf 0 setzen
  ow_akt_delay=0;
  // Timer starten
  TIM_Cmd(TIM7, ENABLE);
  // warten bis Timer abgelaufen
  while(ow_akt_delay<delay);
  // Timer stoppen
  TIM_Cmd(TIM7, DISABLE);
}


//------------------------------------------------------------
// interne Funktion
// Mess-Funktion (us)
// Laenge vom Lo-Impuls messen
//------------------------------------------------------------
uint32_t P_OW_Mess_Lo_us(uint32_t delay)
{
  uint32_t ret_wert=0;
  uint32_t start_pos=0, stop_pos=0;

  // Timer auf 0 setzen
  ow_akt_delay=0;
  // Timer starten
  TIM_Cmd(TIM7, ENABLE);
  // warten bis Timer abgelaufen
  do {
    // Pegel am Data-Pin messen
    if(P_OW_DataRead()==Bit_RESET) {
      if(start_pos==0) start_pos=ow_akt_delay;
    }
    else {
      if(start_pos>0) {
        if(stop_pos==0) stop_pos=ow_akt_delay;
      }
    }
  }while(ow_akt_delay<delay);
  // Timer stoppen
  TIM_Cmd(TIM7, DISABLE);

  // Lo-Impuls ausrechnen
  if(stop_pos>start_pos) {
    ret_wert=stop_pos-start_pos;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// interne Funktion
// Daten-Pin auf Lo-Pegel
//--------------------------------------------------------------
void P_OW_DataLo(void)
{
  ONE_WIRE_PORT->BSRRH = ONE_WIRE_PIN;
}


//--------------------------------------------------------------
// interne Funktion
// Daten-Pin auf Tri-State
//--------------------------------------------------------------
void P_OW_DataHi(void)
{
  ONE_WIRE_PORT->BSRRL = ONE_WIRE_PIN;
}


//--------------------------------------------------------------
// interne Funktion
// Pegel am Daten-Pin auslesen
//  -> bei Lo-Pegel = Bit_RESET
//  -> bei Hi-Pegel = Bit_SET
//--------------------------------------------------------------
BitAction P_OW_DataRead(void)
{
  BitAction ret_wert;

  ret_wert=GPIO_ReadInputDataBit(ONE_WIRE_PORT, ONE_WIRE_PIN);

  return(ret_wert);
}


//------------------------------------------------------------
// Timer-7 ISR
// wird alle 1us aufgerufen
//------------------------------------------------------------
void TIM7_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) {
    // wenn Interrupt aufgetreten
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
 
    ow_akt_delay++;
  }
}

