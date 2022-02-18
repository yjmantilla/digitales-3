/*******************************************************************/
/* Controlador LCD de 4 bits                                       */
/*                                                                 */
/* Por: Luis Germ�n Garc�a Morales                                 */
/*                                                                 */
/* M�dulo:  Controlador de LCD - Cabecera                          */
/* Versi�n: 1.0.0003                                               */
/* Fecha:   29-01-2008                                             */
/*******************************************************************/


/*******************************************************************/

#include "derivative.h"

// Definiciones varias
#define LCD_NOP() __asm NOP; __asm NOP; __asm NOP; __asm NOP; __asm NOP; __asm NOP; __asm NOP; \
                  __asm NOP; __asm NOP; __asm NOP; __asm NOP; __asm NOP; __asm NOP; __asm NOP
                  

#define NOP_SHORT()  __asm NOP; __asm NOP; __asm NOP; __asm NOP

#define NOP_LONG()   NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); \
                     NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); \
                     NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); \
                     NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); \
                     NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); \
                     NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); \
                     NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); \
                     NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); \
                     NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); \
                     NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT(); NOP_SHORT()
                 
// Definiciones para el LCD
#define LCD_NUM_COLS            20
#define LCD_NUM_FILAS            4

// E/S y Pines del puerto C para control del LCD
#define LCD_DIR_CONTROL         PTCDD_PTCDD6
#define LCD_DIR_ENABLE          PTCDD_PTCDD4
#define LCD_DIR_BACKLIGHT       PTCDD_PTCDD2
#define LCD_PIN_CONTROL         PTCD_PTCD6
#define LCD_PIN_ENABLE          PTCD_PTCD4
#define LCD_PIN_BKL             PTCD_PTCD2

// Control E/S de los pines de control de LCD
#define OUT_ENABLE()            LCD_DIR_ENABLE = 1
#define IN_ENABLE()             LCD_DIR_ENABLE = 0
#define OUT_CONTROL_DATA()      LCD_DIR_CONTROL = 1
#define IN_CONTROL_DATA()       LCD_DIR_CONTROL = 0
#define OUT_BACKLIGHT()         LCD_DIR_BACKLIGHT = 1
#define IN_BACKLIGHT()          LCD_DIR_BACKLIGHT = 0

// L�neas de control para el LCD
#define LCD_ENABLE()            LCD_PIN_ENABLE = 1  // Habilitar LCD
#define LCD_DISABLE()           LCD_PIN_ENABLE = 0  // Deshabilitar LCD
#define LCD_CONTROL()           LCD_PIN_CONTROL = 0 // Se�ales de control al LCD
#define LCD_DATA()              LCD_PIN_CONTROL = 1 // Datos al LCD
#define LCD_ON_BKL()            LCD_PIN_BKL = 1     // Habilitar BackLight
#define LCD_OFF_BKL()           LCD_PIN_BKL = 0     // Deshabilitar BackLight

// Direcciones de memoria para el Puerto A (Datos)
#define LCD_ADDRESS_PT          @0x00000000   // Direcci�n puerto A (PTAD)
#define LCD_ADDRESS_DDR         @0x00000001   // Direcci�n puerto A (PTADD)

// Datos del LCD
typedef struct 
{
  byte LSB_Bits  :4;
  byte MSB_Bits  :4;
}LCD_PtcStr; 
extern volatile LCD_PtcStr   _LCD_Port_Data     LCD_ADDRESS_PT; // Direcci�n Puerto 
extern volatile LCD_PtcStr   _LCD_Port_Data_Dir LCD_ADDRESS_DDR; // Direcci�n Puerto
#define LCD_PORT_DATA        _LCD_Port_Data.LSB_Bits       // Usando pines de menor peso de PTA
#define LCD_PORT_DATA_DIR    _LCD_Port_Data_Dir.LSB_Bits   // Usando pines de menor peso de PTA

// Pines del puerto D para datos del LCD
#define OUT_LCD_PORT_DATA()  LCD_PORT_DATA_DIR = 0xF
#define IN_LCD_PORT_DATA()   LCD_PORT_DATA_DIR = 0x0

// Valores al bus de datos cuando se controla el LCD
#define LCD_CLEAR                   0x01
#define LCD_CURATHOME               0x02
#define LCD_ENTRYMODSET             0x04
#define LCD_ONOFFCONTROL            0x08
#define LCD_CURDISPSHIFT            0x10
#define LCD_FUNCTIONSET             0x20
#define LCD_SETADDRCGRAM            0x40
#define LCD_SETADDRDDRAM            0x80
#define LCD_INITIALIZE              0x10

// Entry Mode Set
// Indicar en el par�metro EntryModSet_Cur de la funci�n
#define LCD_ENTRYMODSET_CURINC      0x02
#define LCD_ENTRYMODSET_CURDEC      0x00
// Indicar en el par�metro EntryModSet_Shift de la funci�n
#define LCD_ENTRYMODSET_SHIFTDISP   0x01
#define LCD_ENTRYMODSET_DISPNOSHIFT 0x00

// Display On/Off
// Indicar en el par�metro OnOffControl_Disp de la funci�n
#define LCD_ONOFFCONTROL_DISPON     0x04
#define LCD_ONOFFCONTROL_DISPOFF    0x00
// Indicar en el par�metro OnOffControl_Curs de la funci�n
#define LCD_ONOFFCONTROL_CURSON     0x02
#define LCD_ONOFFCONTROL_CURSOFF    0x00
// Indicar en el par�metro OnOffControl_Blink de la funci�n
#define LCD_ONOFFCONTROL_BLINKON    0x01
#define LCD_ONOFFCONTROL_BLINKOFF   0x00

// Shift
// Indicar en el par�metro CurDispShift de la funci�n
#define LCD_CURDISPSHIFT_DISPSHIFT  0x08
#define LCD_CURDISPSHIFT_CURSMOVE   0x00
// Indicar en el par�metro CurDispShift_LeftRight de la funci�n
#define LCD_CURDISPSHIFT_RIGHTSHIFT 0x04
#define LCD_CURDISPSHIFT_LEFTSHIFT  0x00

// Set Function
// Indicar en el par�metro FunctionSet_Interfaz de la funci�n
#define LCD_FUNCTIONSET_8BITS				0x10
#define LCD_FUNCTIONSET_4BITS				0x00
// Indicar en el par�metro FunctionSet_Line de la funci�n
#define LCD_FUNCTIONSET_2LINE				0x08
#define LCD_FUNCTIONSET_1LINE				0x00
// Indicar en el par�metro FunctionSet_Dots de la funci�n
#define LCD_FUNCTIONSET_5X10				0x04
#define LCD_FUNCTIONSET_5X7 				0x00

// Otras definiciones para el LCD
#define LCD_DELAY_LONG              1640
#define LCD_DELAY_SHORT               42
#define LCD_USE_SECOND_LINE         0x40
#define LCD_USE_TIRTH_LINE          0x14
#define LCD_USE_FOURTH_LINE         0x54

// Variables
extern volatile byte LCD_cnt_BackLight_Time;

// Prototipos de las funciones del LCD
void LCD_Clear(void);
void LCD_CurAtHome(void);
void LCD_EntryModSet(unsigned char, unsigned char);
void LCD_OnOffControl(unsigned char, unsigned char, unsigned char);
void LCD_CurDispShift(unsigned char, unsigned char);
void LCD_FunctionSet(unsigned char, unsigned char, unsigned char);
void LCD_SetAddrCGRAM(unsigned char);
void LCD_SetAddrDDRAM(unsigned char);
void LCD_SetDataRAM(unsigned char);
void LCD_Initialize(unsigned char);

// Otras funciones para el LCD
void LCDWriteMsg(byte, const byte *, word);
void LCDWriteMsgWithoutStart(const byte *, word);
void LCDWriteCenterMsg(byte , const byte *, word);
void LCDWriteReverseCenterMsg(byte, const byte *, word);
void LCDClearLine(byte, word);
void PrintCG_Char(const byte *, byte, byte);
void Print_Personalizated_Chars(void);

void tDelay(word);
void tDelayLarge(word);
void LCD_Init(void);

// Caracteres personalizados
#define LCD_FIRST_CG_CHAR     0
#define LCD_SECOND_CG_CHAR    8
#define LCD_THIRTH_CG_CHAR    16
#define LCD_FOURTH_CG_CHAR    24
#define LCD_FIFTH_CG_CHAR     32
#define LCD_SIXTH_CG_CHAR     40
#define LCD_SEVETH_CG_CHAR    48
#define LCD_EIGHTH_CG_CHAR    56

extern const byte CG_PerChar1[];
extern const byte CG_PerChar2[];
extern const byte CG_PerChar3[];
extern const byte CG_PerChar4[];