#include "timer.h"

void TPM_Init(void) {  
  TPM1SC_PS = 7;
  TPM1MOD = TIMER_MODULE;
  TPM1SC_TOIE = 1;
  TPM1SC_CLKSA = 1;
}

/*void TIMER_Init(void)
{
  T1SC_TSTOP = 1;
  T1SC_TRST = 1;
  T1SC_PS = 6;  // Dividir por 64
  T1SC_TOIE = 1;
  T1MODH = TIMER_MODULE >> 8;
  T1MODL = TIMER_MODULE & 0xFF;
  T1SC_TSTOP = 0;
}*/

interrupt VectorNumber_Vtpm1ovf void TPM1_Ov_ISR(void) {
  (void)TPM1SC;
  TPM1SC_TOF = 0;
}