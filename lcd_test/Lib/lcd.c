/*******************************************************************/
/* Controlador LCD de 4 bits                                       */
/*                                                                 */
/* Por: Luis Germán García Morales                                 */
/*                                                                 */
/* Módulo:  Controlador de LCD - Código Principal                  */
/* Versión: 1.0.0003                                               */
/* Fecha:   29-01-2008                                             */
/*******************************************************************/

#include "lcd.h"
#include "string.h"

// Defina acá los caracteres personalizado que desea
// visualizar en el LCD
const byte CG_GasStat[] = {
  0b00011111, 0b00010001, 0b00010001, 0b00010001,
  0b00011111, 0b00011111, 0b00011111, 0b00011111
};
const byte CG_PacMan[] = {
  0, 0b00001110, 0b00011011, 0b00011111, 
  0b00011000, 0b00011111, 0b00001110, 0
};
const byte CG_Man[] = {
  0b00000100, 0b00001110, 0b00000100, 0b00001110, 
  0b00010101, 0b00010101, 0b00001010, 0b00001010
};
const byte CG_HatHappy[] = {
  0b00001110, 0b00011111, 0, 0b00001010,
  0, 0b00010001, 0b00001110, 0
};

// Manejo del bus de datos del LCD
volatile LCD_PtcStr _LCD_Port_Data;
volatile LCD_PtcStr _LCD_Port_Data_Dir;
volatile byte cnt_BackLight_Time;

// *************************
// Funciones propias del LCD
// *************************

// Función para borrar el Display
void LCD_Clear(void)
{
  LCD_CONTROL();
  LCD_PORT_DATA = (LCD_CLEAR>>4) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  LCD_PORT_DATA = (LCD_CLEAR) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  tDelay(LCD_DELAY_LONG);
}

// Función para llevar el cursor a la primera posición
void LCD_CurAtHome(void)
{
  LCD_CONTROL();
  LCD_PORT_DATA = (LCD_CURATHOME>>4) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  LCD_PORT_DATA = (LCD_CURATHOME) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  tDelay(LCD_DELAY_LONG);
}

/* 
   Función para especificar si el display realiza desplazamientos o no
   EntryModSet_Cur =
    Incrementar posición de cursor: LCD_ENTRYMODSET_CURINC
    Decrementar posición de cursor: LCD_ENTRYMODSET_CURDEC
   EntryModSet_Shift =  
    Desplazar Display:              LCD_ENTRYMODSET_SHIFTDISP
    No desplazar Display:           LCD_ENTRYMODSET_DISPNOSHIFT
*/        
void LCD_EntryModSet(unsigned char EntryModSet_Cur, unsigned char EntryModSet_Shift)
{
  LCD_CONTROL();
  LCD_PORT_DATA = ((LCD_ENTRYMODSET | EntryModSet_Cur | EntryModSet_Shift)>>4) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  LCD_PORT_DATA = (LCD_ENTRYMODSET | EntryModSet_Cur | EntryModSet_Shift) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  tDelay(LCD_DELAY_SHORT);
}

/* 
   Función para encender y apagar determinados elementos del LCD
   OnOffControl_Disp =
    Encender Display:   LCD_ONOFFCONTROL_DISPON
    Apagar Display:     LCD_ONOFFCONTROL_DISPOFF
   OnOffControl_Curs =
    Encender Cursor:    LCD_ONOFFCONTROL_CURSON
    Apagar Cursor:      LCD_ONOFFCONTROL_CURSOFF
   OnOffControl_Blink =
    Cursor Parpadea:    LCD_ONOFFCONTROL_BLINKON
    Cursor no Parpadea: LCD_ONOFFCONTROL_BLINKOFF
*/        
void LCD_OnOffControl(unsigned char OnOffControl_Disp, unsigned char OnOffControl_Curs, unsigned char OnOffControl_Blink)
{
  LCD_CONTROL();
  LCD_PORT_DATA = ((LCD_ONOFFCONTROL | OnOffControl_Disp | OnOffControl_Curs | OnOffControl_Blink)>>4) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  LCD_PORT_DATA = (LCD_ONOFFCONTROL | OnOffControl_Disp | OnOffControl_Curs | OnOffControl_Blink) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  tDelay(LCD_DELAY_SHORT);
}

/* 
   Función para desplazar el display o el cursor a izquierda o derecha
   CurDispShift =  
    Desplazar Display:     LCD_CURDISPSHIFT_DISPSHIFT
    Desplazar Cursor:      LCD_CURDISPSHIFT_CURSMOVE
   CurDispShift_LeftRight =  
    Desplazar a derecha:   LCD_CURDISPSHIFT_RIGHTSHIFT
    Desplazar a izquierda: LCD_CURDISPSHIFT_LEFTSHIFT
*/        
void LCD_CurDispShift(unsigned char CurDispShift, unsigned char CurDispShift_LeftRight)
{
  LCD_CONTROL();
  LCD_PORT_DATA = ((LCD_CURDISPSHIFT | CurDispShift | CurDispShift_LeftRight)>>4) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  LCD_PORT_DATA = (LCD_CURDISPSHIFT | CurDispShift | CurDispShift_LeftRight) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  tDelay(LCD_DELAY_SHORT);
}

/* 
   Función para indicar características del display como bits de interfaz, entre otros
   FunctionSet_Interfaz =
    Interfaz 8 Bits:     LCD_FUNCTIONSET_8BITS
    Interfaz 4 Bits:     LCD_FUNCTIONSET_4BITS
   FunctionSet_Line =
    Usar dos líneas:     LCD_FUNCTIONSET_2LINE
    usar una línea:      LCD_FUNCTIONSET_1LINE
   FunctionSet_Dots = 
    Resolución caracter: LCD_FUNCTIONSET_5X10
    Resolución caracter: LCD_FUNCTIONSET_5X7
*/        
void LCD_FunctionSet(unsigned char FunctionSet_Interfaz, unsigned char FunctionSet_Line, unsigned char FunctionSet_Dots)
{
  LCD_CONTROL();
  LCD_PORT_DATA = ((LCD_FUNCTIONSET | FunctionSet_Interfaz | FunctionSet_Line | FunctionSet_Dots)>>4) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  LCD_PORT_DATA = (LCD_FUNCTIONSET | FunctionSet_Interfaz | FunctionSet_Line | FunctionSet_Dots) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  tDelay(LCD_DELAY_SHORT);
}

/* 
   Función para seleccionar la memoria de caracteres donde se lee/escribe
   Address: Dirección de memoria de caracteres
*/        
void LCD_SetAddrCGRAM(unsigned char Address)
{
  LCD_CONTROL();
  LCD_PORT_DATA = ((LCD_SETADDRCGRAM | (Address & 0x3F))>>4) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  LCD_PORT_DATA = (LCD_SETADDRCGRAM | (Address & 0x3F)) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  tDelay(LCD_DELAY_SHORT);
}

/* 
   Función para seleccionar la memoria de visualización donde se lee/escribe
   Address: Dirección de memoria de datos
*/        
void LCD_SetAddrDDRAM(unsigned char Address)
{
  LCD_CONTROL();
  LCD_PORT_DATA = ((LCD_SETADDRDDRAM | (Address & 0x7F))>>4) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  LCD_PORT_DATA = (LCD_SETADDRDDRAM | (Address & 0x7F)) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  tDelay(LCD_DELAY_SHORT);
}

/* 
   Función para escribir un dato en la RAM
   Character: Caracter a imprimir en el LCD o a guardar en la memoria de caracteres
*/        
void LCD_SetDataRAM(unsigned char Character)
{
  LCD_DATA();
  LCD_PORT_DATA = (Character>>4) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  LCD_PORT_DATA = (Character) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
  tDelay(LCD_DELAY_SHORT);
}

// Función para inicializar el LCD
void LCD_Initialize(unsigned char Initialize_Val)
{
  LCD_CONTROL();
  LCD_PORT_DATA = ((LCD_FUNCTIONSET | Initialize_Val)>>4) & 0x0F;
  LCD_ENABLE();
  LCD_NOP();
  LCD_DISABLE();
}

// *******************************************************
// Funciones para escribir en el LCD usando las primitivas
// *******************************************************

// Función para escribir un caracter personalizado en el LCD
void PrintCG_Char(const byte * perChar, byte startAddr, byte showAddr) {
  byte i = 0;
  LCD_SetAddrCGRAM(startAddr);
  for(i=0;i<8;i++)
    LCD_SetDataRAM(*(perChar++));
  LCD_SetAddrDDRAM(showAddr);
  LCD_SetDataRAM(startAddr >> 3);
}

// Función para escribir un mensaje en la memoria del LCD
void LCDWriteMsg(byte StartCurs, const byte *Msg, word msDelay)
{
	// Imprimir en la primera o segunda línea
	LCD_SetAddrDDRAM(StartCurs);
		
	while (*Msg!='\0')
	{
		LCD_SetDataRAM(*(Msg++));
		if (msDelay > 0)
		  tDelayLarge(msDelay);
	}
}

// Función para escribir un mensaje en la memoria del LCD donde esté
void LCDWriteMsgWithoutStart(const byte *Msg, word msDelay)
{
	while (*Msg!='\0')
	{
		LCD_SetDataRAM(*(Msg++));
		if (msDelay > 0)
		  tDelayLarge(msDelay);
	}
}

// Función para escribir un mensaje en la memoria del LCD centrado
void LCDWriteCenterMsg(byte StartCurs, const byte *Msg, word msDelay)
{
	unsigned char nPos;
	nPos = LCD_NUM_COLS/2 - strlen(Msg)/2 + StartCurs;
	
	// Imprimir en la primera o segunda línea
	LCD_SetAddrDDRAM(nPos);
		
	while (*Msg!='\0')
	{
		LCD_SetDataRAM(*(Msg++));
		if (msDelay > 0)
		  tDelayLarge(msDelay);
	}
}

// Función para escribir un mensaje en la memoria del LCD centrado, en reversa
void LCDWriteReverseCenterMsg(byte StartCurs, const byte *Msg, word msDelay)
{
	unsigned char nPos;
	nPos = LCD_NUM_COLS/2 - strlen(Msg)/2 + StartCurs;
	
	// Imprimir en la primera o segunda línea
	LCD_SetAddrDDRAM(nPos + strlen(Msg));
  LCD_EntryModSet(LCD_ENTRYMODSET_CURDEC, LCD_ENTRYMODSET_DISPNOSHIFT);
		
	while (*Msg!='\0')
	{
		LCD_SetDataRAM(*(Msg++));
		if (msDelay > 0)
		  tDelayLarge(msDelay);
	}
  LCD_EntryModSet(LCD_ENTRYMODSET_CURINC, LCD_ENTRYMODSET_DISPNOSHIFT);
}

// Función para borrar líneas del Display
void LCDClearLine(byte StartLine, word msDelay)
{
	byte i;
	LCD_SetAddrDDRAM(StartLine);
	for (i=0;i<LCD_NUM_COLS;i++)
	{
		LCD_SetDataRAM(' ');
		if (msDelay > 0)
		  tDelayLarge(msDelay);
	}
}

// Función para escribir caracteres personalizados
void Print_Personalizated_Chars(void) {
  LCD_Clear();
  PrintCG_Char(&CG_GasStat[0], LCD_FIRST_CG_CHAR, 8);
  PrintCG_Char(&CG_PacMan[0], LCD_SECOND_CG_CHAR, 73);
  PrintCG_Char(&CG_Man[0], LCD_THIRTH_CG_CHAR, 30);
  PrintCG_Char(&CG_HatHappy[0], LCD_FOURTH_CG_CHAR, 95);
}

// Función que se encarga de imprimir un mensaje, lo que cabe de él


// ***********************************
// Funciones para la operación del LCD
// ***********************************

// Esta función puede generar hasta 65.5 ms (base 1us)
void tDelay(word us) {
 
  _asm {
          PSHH;
          PSHX;
  Delay:  LDHX  us;
  _delay: AIX   #-1;    // 2
          CPHX  #0;     // 3
          NOP;          // 4
          NOP;     
          NOP;
          NOP; 
          BNE   _delay; // 3
          PULX;
          PULH;
  }
}

// Esta función puede generar hasta 65.5 ms (base 400us)
void tDelayLarge(word ms) {
  word temp = (word)(5*((dword)ms));
  _asm {
          PSHH;
          PSHX;
  Delay:  LDHX  temp;
  _delay: AIX   #-1;    // 2
          CPHX  #0;     // 3
  }
          NOP_LONG();NOP_LONG();NOP_LONG(); // 2400
          NOP_LONG();NOP_LONG();NOP_LONG();
 _asm {
          BNE   _delay; // 3
          PULX;
          PULH;
  }
}

// Función de inicialización
void LCD_Init(void)
{
  // Puerto de Control y Datos
  OUT_ENABLE();         // Habilitar salida para Enable
  OUT_CONTROL_DATA();   // Habilitar salida para Control/Datos
  OUT_BACKLIGHT();      // Habilitar salida para Backlight
  OUT_LCD_PORT_DATA();  // Habilitar salida para bus de datos
  
  // Datos Iniciales
  LCD_CONTROL();
  LCD_DISABLE();
  LCD_OFF_BKL();
  LCD_PORT_DATA = 0;
  
  // Inicialización del Display
	// Generar pausa de 15 ms
	tDelay(15000);
	// Mandar primer comando de inicialización y esperar 4.1 ms
	LCD_Initialize(LCD_INITIALIZE);
	tDelay(4100);		
	// Mandar segundo comando de inicialización y esperar 100 us
	LCD_Initialize(LCD_INITIALIZE);
	tDelay(100);		
	// Mandar tercer comando de inicialización de y no esperar
	LCD_Initialize(LCD_INITIALIZE);
	tDelay(100);		
	
  // Seleccionar número de líneas y puntos por caracter
  LCD_FunctionSet(LCD_FUNCTIONSET_4BITS, LCD_FUNCTIONSET_2LINE, LCD_FUNCTIONSET_5X10);
  // Encender LCD y Apagar el Cursor
  LCD_OnOffControl(LCD_ONOFFCONTROL_DISPON, LCD_ONOFFCONTROL_CURSOFF, LCD_ONOFFCONTROL_BLINKOFF);
  // Borrar todo el LCD
  LCD_Clear();
  // Establecer desplazamiento del cursor
  LCD_EntryModSet(LCD_ENTRYMODSET_CURINC, LCD_ENTRYMODSET_DISPNOSHIFT);
}

