#include "derivative.h"
#include "general.h"

// Inicialización del Sistema
void SYSTEM_INIT(void)
{
	SOPT1 = 0x20;
}

// Inicialización del MCG (Fbus = 12MHz)
void MCG_Init(void) {  
// Moving from FEI to PEE Mode: External Crystal = 12 MHz,Bus Frequency = 12 MHz
//1. First, FEI must transition to FBE mode:
  MCGC2 = MCGC2_RANGE_MASK | MCGC2_HGO_MASK | MCGC2_EREFS_MASK | MCGC2_ERCLKEN_MASK;
          // BDIV (bits 7 and 6) set to %00, or divide-by-1                                      
          // RANGE (bit 5) set to 1 because the frequency of 12 MHz is within the high frequency range                                       
          // HGO (bit 4) set to 1 to configure external oscillator for high gain operation                                      
          // EREFS (bit 2) set to 1, because a crystal is being used                                      
          // ERCLKEN (bit 1) set to 1 to ensure the external reference clock is active                                      
  while(MCGSC_OSCINIT!=1){};                      
          // crystal selected by the EREFS bit has been initialized
  MCGC1 = MCGC1_CLKS1_MASK|MCGC1_RDIV_MASK;     
          // CLKS (bits 7 and 6) set to %10 in order to select external reference clock as system clock source
          // RDIV (bits 5-3) set to %111, or divide-by-128.
          // IREFS (bit 2) cleared to 0, selecting the external reference clock
  while (MCGSC_IREFST!=0){};
          // external reference is the current source for the reference clock
  while (MCGSC_CLKST!=0b10){};                       
          // external reference clock is selected to feed MCGOUT
//2. Then, FBE mode transitions into BLPE mode:
    MCGC2 |= MCGC2_LP_MASK;
          // LP (bit 3) in MCGC2 to 1 (BLPE mode entered)  
    MCGC1_RDIV = 0b011;
          // RDIV (bits 5-3) set to %011, or divide-by-8 because 12 MHz / 8= 1.5 MHz which is in the 1                         
          // MHz to 2 MHz range required by the PLL.
    MCGC3 = MCGC3_PLLS_MASK|MCGC3_VDIV2_MASK;   
          // PLLS (bit 6) set to 1, selects the PLL. In BLPE mode,
          // VDIV (bits 3-0) set to %0100, or multiply-by-16 because                                      
	while (MCGSC_PLLST!=1){};                       
	       // current source for the PLLS clock is the PLL
//3. Then, BLPE mode transitions into PBE mode:	
	MCGC2_LP =0; 
	      // Clear LP (bit 3) in MCGC2 to 0 here to switch to PBE mode                                 
	while (MCGSC_LOCK==0){};
	      // PLL has acquired lock                  
	MCGC1_CLKS = 0b00;
	      // CLKS (bits7 and 6) in MCGSC1 set to %00 in order to select the output of the PLL as the system clock source
	while(MCGSC_CLKST != 0b11){};                       
        // PLL output is selected to feed MCGOUT in the current clock mode
        // Now, With an RDIV of divide-by-8, a BDIV of divide-by-1, and a VDIV of multiply-by-16,
        // MCGOUT = [(12 MHz / 8) * 16] / 1 = 24 MHz, and the bus frequency is MCGOUT / 2, or 12 MHz
}