/* 
 * Ejemplo de I2C. Usando el Driver Estándar
   Manejo de la Memoria 24LC16. Páginas de 16 datos.
   La aplicación es muy simple, un conjunto de valores se escriben en la memoria y luego se
   leen. Se comparan los datos escritos con los leidos. Finalmente se envían
   mensajes al LCD de acuerdo a la ejecución.
   Los datos se escriben al final de la página 16, sabiendo que la primera página es la cero
   es decir, en la dirección 0x10F (memoria 24LC16 con 16 bytes por páginas). El Driver
   automáticamente hace el salto de página para no perder datos.
 
   Este ejemplo también configura el RTC y lo lee cada 0.5seg.

   Este programa no hace uso de metodología de eventos, es
   simplemente una ilustración de como emplear el protocolo I2C.
   
   ALAMBRADA (LCD)   
   
              PTC6 PTC4                          PTA0 PTA1 PTA2 PTA3 VCC
    _|____|____|____|____|____|____|____|____|____|____|____|____|____|__
   |           RS   E                             D4   D5   D6   D7   BL |
   | 1                                                                16 |
   |         LCD                                                         |
   |_____________________________________________________________________|
   
   ALAMBRADA (I2C)
   
      PTA4    PTA5
    ___|_______|___
   |  SDA     SCL  |
   |               |
   |_______________|
   
   */
   

#include "derivative.h" 
#include "driver_i2c.h"
#include "general.h"
#include "lcd.h"
#include "timer.h"

// Dirección del periférico
#define EEPROM        0x50  // 1010000 = 1 0 1 0 B2 B1 B0
#define RTC           0x68  // 1101000

#define NUMOFDATS     48
#define NUMBYTESINMEM 2048

// Valores a escribir en EEPROM
const byte ValuesToWrite[NUMOFDATS] =
{
  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
  17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
  33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48
};
// Zona de memoria donde se guardan los datos leidos desde la EEPROM
byte ValuesToRead[NUMOFDATS] = 0;

// Mensajes en LCD
const byte lcdMsg1[] = "EEPROM y RTC";
const byte lcdMsg2a[] = "Clearing Mem:";
const byte lcdMsg2[] = "Writing: ";
const byte lcdMsg3[] = "Reading: ";
const byte lcdMsg4[] = "Comparing: ";
const byte lcdMsg5[] = "Fecha:  __/__/__";
const byte lcdMsg6[] = "Hora:   __:__:__";
#define FIRST_CHAR_DATE     8 
const byte lcdMsgOk[] = "Ok";
const byte lcdMsgEEPROMError[] = "Error";
const byte lcdMsgRTCError[] = "RTC Error";
// Función principal
void main(void) 
{
  // Variables
  word wAddressWrite = 0x10F;  // Al final de la página 16 (Página 0, 1, 2...)
  word wAddressRead = 0;
  byte i;
  byte Status;

  // Códigos de Error de la Memoria y RTC
  #define NO_ERROR      0
  #define ERROR_CLEAR   1
  #define ERROR_WRITE   2
  #define ERROR_READ    3
  #define ERROR_COMPARE 4
  #define ERROR_RTC     5

  // Inicialización
  SYSTEM_INIT();
  MCG_Init();
  LCD_Init();
  TPM_Init();

  CLI;
  
  // Mensajes de inicialización
  LCDWriteCenterMsg(0,lcdMsg1,0);

  // Debido a que la única interrupción es la del TIMER, se esperan 10 de ellas
  // para generar 5 segundos de retraso.
  #define SEG_WAIT  10
  for (i=0;i<SEG_WAIT;i++) 
    WAIT;
	
  // Borrar toda la memoria
  LCD_Clear();
  LCDWriteMsg(0,lcdMsg2a,0);
  if (Fill_Data_EEPROM_I2C(EEPROM, 0, NUMBYTESINMEM, 0x00, MEMORYSIZE_2048BYTES, PAGESIZE_16BYTES) == FALSE)
    Status = ERROR_CLEAR;
  else
  {
    // Escribir en EEPROM
    LCD_Clear();
    LCDWriteMsg(0,lcdMsg2,0);
    if (Write_Data_EEPROM_I2C(EEPROM, &ValuesToWrite[0], NUMOFDATS, wAddressWrite, MEMORYSIZE_2048BYTES, PAGESIZE_16BYTES) == FALSE)
      Status = ERROR_WRITE;
    else
    {
      // Lectura de datos.
      wAddressRead = wAddressWrite;
      LCD_Clear();
      LCDWriteMsg(0,lcdMsg3,0);
    	if (Read_Data_EEPROM_I2C(EEPROM, &ValuesToRead[0], NUMOFDATS, wAddressRead, MEMORYSIZE_2048BYTES) == FALSE)
    	  Status = ERROR_READ;
    	else
    	{
    	  // Comparar datos
    	  Status = NO_ERROR; // Asumir que no hay error
        LCD_Clear();
        LCDWriteMsg(0,lcdMsg4,0);
    	  for (i=0;i<NUMOFDATS;i++)
    	  {
    	    if (ValuesToRead[i] != ValuesToWrite[i])
    	    {
    	      Status = ERROR_COMPARE;
    	      break;
    	    }
    	  }
    	}
    }    
  }

  // Validar si hubo errores para ajustar el módulo del TIMER
  switch (Status)
  {
    case ERROR_CLEAR:
    case ERROR_WRITE:
      LCDWriteMsgWithoutStart(lcdMsgEEPROMError,0);
      break;
    case ERROR_READ:
      LCDWriteMsgWithoutStart(lcdMsgEEPROMError,0);
      break;
    case ERROR_COMPARE:
      LCDWriteMsgWithoutStart(lcdMsgEEPROMError,0);
      break;
    case NO_ERROR:
      LCDWriteMsgWithoutStart(lcdMsgOk,0);
      break;
    default:
      break;
  }

  // Mensajes de inicialización, se esperan 5 segundos
  for (i=0;i<SEG_WAIT;i++) 
    WAIT;

  // Si no hubo errores, configurar RTC
  if (Status == NO_ERROR)
  {
    LCDWriteMsg(0,lcdMsg4,0);
    // Fecha y hora para el RTC
    RTC_CH = 0;
    RTC_UNI_SEG = 0;
    RTC_DEC_SEG = 5;
    RTC_UNI_MIN = 9;
    RTC_DEC_MIN = 5;
    RTC_12_24 = RTC_FORMAT_24H;
    RTC_UNI_HOUR24 = 3;
    RTC_DEC_HOUR24 = 2;
    RTC_DIASEM = 1;
    RTC_UNI_DIA = 9;
    RTC_DEC_DIA = 2;
    RTC_UNI_MES = 2;
    RTC_DEC_MES = 0;
    RTC_UNI_YEA = 1;
    RTC_DEC_YEA = 0;
    if (Write_Data_RTC_I2C(RTC, (unsigned char *)&_var_rtc, 7, 0x00) == FALSE)
    {
      LCDWriteMsg(LCD_USE_SECOND_LINE,lcdMsgRTCError,0);
      Status = ERROR_RTC;
    }
    else
    {
      LCDWriteMsg(0,lcdMsg5,0);
      LCDWriteMsg(LCD_USE_SECOND_LINE,lcdMsg6,0);
    }
  }

  for (;;)
  {
    WAIT;
  
    // Refrescar RTC cuando haya interrupción de TIMER
    if (Status == NO_ERROR)
    {
      if (Read_Data_RTC_I2C(RTC, (unsigned char *)&_var_rtc, 7, 0x00) == FALSE)
      {
        LCDWriteMsg(0,lcdMsg5,0);
        LCDWriteMsg(LCD_USE_SECOND_LINE,lcdMsg6,0);
      }
      else
      {
        ValuesToRead[0] = RTC_DEC_DIA | 0x30;
        ValuesToRead[1] = RTC_UNI_DIA | 0x30;
        ValuesToRead[2] = '/';
        ValuesToRead[3] = RTC_DEC_MES | 0x30;
        ValuesToRead[4] = RTC_UNI_MES | 0x30;
        ValuesToRead[5] = '/';
        ValuesToRead[6] = RTC_DEC_YEA | 0x30;
        ValuesToRead[7] = RTC_UNI_YEA | 0x30;
        ValuesToRead[8] = '\0';
        LCDWriteMsg(FIRST_CHAR_DATE,ValuesToRead,0);
        
        ValuesToRead[0] = RTC_DEC_HOUR24 | 0x30;
        ValuesToRead[1] = RTC_UNI_HOUR24 | 0x30;
        ValuesToRead[2] = ':';
        ValuesToRead[3] = RTC_DEC_MIN | 0x30;
        ValuesToRead[4] = RTC_UNI_MIN | 0x30;
        ValuesToRead[5] = ':';
        ValuesToRead[6] = RTC_DEC_SEG | 0x30;
        ValuesToRead[7] = RTC_UNI_SEG | 0x30;
        ValuesToRead[8] = '\0';
        LCDWriteMsg(LCD_USE_SECOND_LINE+FIRST_CHAR_DATE,ValuesToRead,0);
      }
      
    }
  }
}
