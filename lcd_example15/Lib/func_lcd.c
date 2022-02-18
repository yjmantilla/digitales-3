#include "derivative.h"
#include <string.h>
#include "lcd.h"
#include "general.h"

// *****************************************************
// Funciones para enviar mensajes al controlador del LCD
// *****************************************************

const byte Msg1[] = "ELECTRONIC.DIGITAL3!";
const byte Msg2[] = "Prueba de LCD";
const byte Msg3[] = "Is back light on?";
const byte Msg4[] = "TIME IS NOW";
void PrintMSG_Static(void)
{
   LCD_Clear();
   LCDWriteMsg(0,Msg1,0);
   LCDWriteMsg(LCD_USE_SECOND_LINE,Msg2,0);
   LCDWriteMsg(LCD_USE_TIRTH_LINE,Msg3,0);
   LCDWriteMsg(LCD_USE_FOURTH_LINE,Msg4,0);
}

const byte MsgCenter1[] = "Centrado ";
const byte MsgCenter2[] = "... By now ...";
void PrintMSG_StaticCenter(void)
{
   LCD_Clear();
   LCDWriteCenterMsg(0,MsgCenter1,0);
   LCDWriteCenterMsg(LCD_USE_TIRTH_LINE,MsgCenter2,0);
}

const byte MsgDyn1[] = "Are you bored ?";
const byte MsgDyn2[] = "And hungry?";
void PrintMSG_Dynamic(void)
{
   LCDWriteCenterMsg(LCD_USE_SECOND_LINE,MsgDyn1,100);
   LCDWriteCenterMsg(LCD_USE_FOURTH_LINE,MsgDyn2,100);
}

void PrintMSG_ClearLine(void)
{
   LCDClearLine(LCD_USE_SECOND_LINE,100);
   LCDClearLine(LCD_USE_FOURTH_LINE,100);
}

const byte MsgDynamic2[] = "Electronic. Digital3 Ing.Electronica";
void PrintMSG_Dynamic2(void)
{
   LCDWriteMsg(LCD_USE_SECOND_LINE,MsgDynamic2,100);
}

void PrintMSG_DespLCD(void)
{
   byte lenMsg = (byte) strlen(MsgDynamic2);
   byte i;
   
   for (i=0;i<strlen(MsgDynamic2)-LCD_NUM_COLS;i++)
   {
      LCD_CurDispShift(LCD_CURDISPSHIFT_DISPSHIFT, LCD_CURDISPSHIFT_LEFTSHIFT);
      tDelayLarge(100);
   }
}
