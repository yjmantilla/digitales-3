/***************************************************************************
   Este programa muestra como hacer uso de un LCD, empleando una interfaz
   de 4-bits para los datos.
   
   ALAMBRADA (JM60)   
   
              PTC6 PTC4                          PTA0 PTA1 PTA2 PTA3 VCC
    _|____|____|____|____|____|____|____|____|____|____|____|____|____|__
   |           RS   E                             D4   D5   D6   D7   BL |
   | 1                                                                16 |
   |         LCD                                                         |
   |_____________________________________________________________________|
          
****************************************************************************/
   
#include <hidef.h>
#include "derivative.h"
#include "lcd.h"
#include "timer.h"
#include "general.h"
#include "MCG.h"

word cntTimer = CNT_TIMER;

// Funci�n principal del programa.
void main(void) 
{  
  byte cnt_Func = 0;
  
  // Inicializaci�n del LCD y TIMER
  SOPT1 = 0x20;
  MCG_Init();
  LCD_Init();
  TPM_Init();  
  //PTBD_PTBD4 = 0;
  //PTBDD_PTBDD4 = 1;
  // For infinito
  Print_Personalizated_Chars();
  for(;;) 
  {
    // Entra a modo WAIT, una interrupci�n de TIMER lo saca.
   asm WAIT;
    
    // Cada CNT_TIMER segundos, se cambia de estado.
    // cnt_Func controla el estado de la F.S.M.
    if (--cntTimer == 0)
    {
      cntTimer = CNT_TIMER;

      // switch (cnt_Func)
      // {
      //   case 0:
      //     PrintMSG_Static();
      //     cnt_Func++;
      //     break;
      //   case 1:
      //     PrintMSG_StaticCenter();
      //     cnt_Func++;
      //     break;
      //   case 2:
      //     PrintMSG_Dynamic();
      //     cnt_Func++;
      //     break;
      //   case 3:
      //     PrintMSG_ClearLine();
      //     cnt_Func++;
      //     break;
      //   case 4:
      //     PrintMSG_Dynamic2();
      //     cnt_Func++;
      //     break;
      //   case 5:
      //     Print_Personalizated_Chars();
      //     cnt_Func=0;
      //     break;
      // }
    }
  }
}
