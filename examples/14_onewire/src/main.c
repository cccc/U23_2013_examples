#include <System.h>
#include <stdio.h>
#include <stm32_ub_ds18xx.h>
#include "main.h"

//Pins: Minus kommt an Ground, das in der Mitte ist VDD und das was übrig bleibt ist das andere!!

int main(void)
{
  InitializeSystem();
  EnableDebugOutput(DEBUG_ITM);

  float temp;
  ErrorStatus check;
  char buf[20];
 
  SystemInit(); // Quarz Einstellungen aktivieren
 
  // init vom DS1822
  check=UB_DS18XX_Init();
 
  if(check==ERROR) {
    printf("error\n");
  }

  if(UB_OneWire_ReadRomCode())
  {
    printf("Romcode lesen hat funktioniert\n");
  } 
  else 
  {
    printf("Romcode lesen hat nicht funktioniert\n");
  }

  for (int i = 0; i < 8; i++)
  {
    printf("%02x", one_wire_rom_code[i]);
  } 
  printf("\n");
  
  while(1)
  {
    if(check==SUCCESS) {
      // kleine Pause
      Delay(1);
      // Temperatur vom DS1822 auslesen
      temp=UB_DS1822B20_ReadTemp("288bf865050000b0");
     
      printf("%f\n", temp);
    }
  }
}
