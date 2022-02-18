#include "derivative.h" // Registros del MCU
#include "timer.h"      // Variables y prototipos de general.c

// Funci�n de inicializaci�n del TPM1
void TPM_Init(void) {
  TPM1SC_PS = 7;
  TPM1MOD = TIM_MODULE;
  TPM1SC_TOIE = 1;
  TPM1SC_CLKSA = 1;
}

// Subrutina de atenci�n a interrupci�n del TPM1 Overflow
interrupt VectorNumber_Vtpm1ovf void TPM1_Ov_ISR(void) {
  (void)TPM1SC;
  TPM1SC_TOF = 0;
}
