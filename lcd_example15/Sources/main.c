// Headers files for accessing the MCU registers
#include <hidef.h>
#include "derivative.h"

#define MODULO 30000//VALUE MODULE REGISTER

//VariablEs for counting and indicating TIMER event.
// Volatile is useful for memory-mapped IO, prevents compiler optimization - avoid caching it, indicates the variable may change by outside factors
volatile unsigned char Flag_Int_IC=0;//variable for indicating to main program that an Input Capture(IC) has ocurred
volatile unsigned char Flag_Int_OC=0;//variable for indicating to main program that an Output Compare(OC) has ocurred
//volatile unsigned char entro =0;
unsigned int time_edge_1, time_edge_2;//variables for storing ocurrence of a rising or falling edge
unsigned long  time_pulse_width;//variable for indicating pulse width
volatile unsigned int  count_Ovf=0; //variable for indicating overflow count

/*
 * IMPORTANT!
 * **EVERY TIMER MODULE ONLY HAS ONE MODULE REGISTER
 * 
 * **DON'T FORGET THE DEBOUNCES FOR INPUT CAPTURE
 * 
 * */

// ACK --> Remove the interruption from the queue

void TIMER1_Init(void)// TIMER MODULE FUNCTION INITIALIZATION
{
	//TPMxSC = TPM Status and Control Register, x is either one of the timers (there are two)
	
	// Clock Source Selects
	// NO CLOCK SELECTED (TPM1 DISABLED) --> This is just to clear and later set them?
	TPM1SC_CLKSB=0;
	TPM1SC_CLKSA=0;
	
	// TOIE = Timer overflow interrupt enable, local mask to enable interrupts by the timer
	TPM1SC_TOIE=1;//INTERRUPT ENABLE (OVERFLOW). Local mask
	
	// CPWMS = Center-aligned PWM select
	TPM1SC_CPWMS=0;//ALL CHANNELS OPERATE AS I.C, O.C, edge-aligned PWM
	
	// PS = Prescale factor select
	// BUS CLOCK = 2.5 MHz---> 0.4us clock periodo
	TPM1SC_PS=1;//TPM Clock Source Divides-by 128 --> 51.2us clock period
	
	TPM1MOD= MODULO;//MODULE for overflow counts
		
	
	/////////////////// CHANNEL 0 //////////////////////////////////
	// Channel n interrupt enable, to be able to do interruptions from the timer
	TPM1C0SC_CH0IE=1;//INTERRUPT ENABLE
	
	//CONFIGURE CHANNEL 0 AS I.C ->PTE2
	
	//MODE INPUT CAPTURE
	// 16.4.2.1 Input Capture Mode
	// Page 287
	
	//Mode select bits --> Pag 283 Table 16-6. Mode, Edge, and Level Selection
	// 00 for Input capture 
	TPM1C0SC_MS0B=0;
	TPM1C0SC_MS0A=0;
	
	//CONFIGURE CAPTURE ON RISING OR FALLING EDGE (see the same table)
	TPM1C0SC_ELS0B=1;
	TPM1C0SC_ELS0A=1;
	// 11 = Capture on rising or falling edge

	/////////////// CHANNEL 1 //////////////////////////////////////////
	//CONFIGURE CHANNEL 1 AS O.C.  ->PTE3
	
	//16.4.2.2 Output Compare Mode
	// Why output-compare? Because an output is done when the counter of the timer is compared to a desired value and it is equal
	//With the output-compare function, the TPM can generate timed pulses with programmable position,
	//polarity, duration, and frequency. When the counter reaches the value in the channel-value registers of an
	//output-compare channel, the TPM can set, clear, or toggle the channel pin.
	TPM1C1SC_CH1IE=0;//INTERRUPT DISABLE
	//MODE OUTPUT CAPTURE
	TPM1C1SC_MS1B=0;
	TPM1C1SC_MS1A=1;
	//SET OUTPUT ON COMPARE
	// 3 submodes --> toggle , clear, set. Here we use 11=set
	TPM1C1SC_ELS1B=1; 
	TPM1C1SC_ELS1A=1;
	
	//TPMxCnV
	TPM1C1V=5000;//PULSE WIDTH
	//THE PERIOD OF THE SIGNAL IS DEFINED BY THE MODULE REGISTER
	
	// After configuring the timer we end with setting the clock
	// Enable system clock --> 01=Bus rate clock
	TPM1SC_CLKSB=0;// BUS RATE CLOCK
	TPM1SC_CLKSA=1;
}



interrupt 9 void TPM1_IC_ISR(void){//INTERRUPT ENABLE, priority 9
	// CLEAR CH0F IN TWO STEPS:
	// Page 282, Table 16-5. TPMxCnSC Field Descriptions
	// Clear CHnF by reading TPMxCnSC while CHnF is set and then writing a logic 0 to CHnF.
	TPM1C0SC;// 1.READING TPMxSC WHEN TOF IS SET
	TPM1C0SC_CH0F = 0; // 2. WRITING A LOGIC 0 TO CH0F
	Flag_Int_IC=1;
}

interrupt 10 void TPM1_OC_ISR(void){//INTERRUPT DISABLE
	// CLEAR CH1F IN TWO STEPS:
	TPM1C1SC;// 1.READING TPMxSC WHEN TOF IS SET
	TPM1C1SC_CH1F = 0; // 2. WRITING A LOGIC 0 TO CH2F
	Flag_Int_OC=1;
}

interrupt 15 void TPM1_OvF_ISR(void){//INTERRUPT ENABLE
	// CLEAR TOF IN TWO STEPS:
	TPM1SC;// 1.READING TPMxSC WHEN TOF IS SET
	
	// TOF --> Timer Overflow Flag
	TPM1SC_TOF = 0; // 2. WRITING A LOGIC 0 TO TOF
	count_Ovf++;  
	 
}

// PORT FUNCTION INITIALIZATION
void Port_Init(void)
{
	PTADD=0xFF;//OUT FOR LEDS
	PTAD=0x00;
	PTDD = 0x00;
	PTDDD = 0x00;
	PTED = 0x00;
	PTEDD = 0x00;


}

//MAIN FUNCTION
void main(void) 
{
  // VARIABLES DECLARATIONS
  char flag=0;//VARIABLE FOR INDICATING A FIRST EDGE OR SECOND EDGE
  
  //SYSTEM INITIALIZATION
  SOPT1_COPT = 0;
  Port_Init();
  TIMER1_Init();

  // ENABLE INTERRUPTS 
  EnableInterrupts; 
  Flag_Int_IC=0;
  for(;;) 
  {
    __asm WAIT;
    

    if(Flag_Int_IC==1){
    	Flag_Int_IC=0;//DON'T FORGET TO TURN OFF THE FLAG

    	if (flag==0){//FIRST EDGE
    		count_Ovf=0;
    		time_edge_1=TPM1C0V;//CAPTURE THE TIME INDICATING A RISING OR FALLING EDGE
    		flag=1;//SECOND EDGE
    		PTAD_PTAD0=1;//LEDS
    		PTAD_PTAD1=0;//LEDS
    		PTAD_PTAD2=1;
    	}
    	else if(flag==1){//SECOND EDGE
    		time_edge_2=TPM1C0V;//CAPTURE THE TIME INDICATING THE OTHERWISE
    		flag=0;//FISRT EDGE (NEW PULSE)
    		PTAD_PTAD0=0;//LEDS
    		PTAD_PTAD1=1;//LEDS
    		PTAD_PTAD2=0;
    		time_pulse_width=(long)(count_Ovf*30000+time_edge_2-time_edge_1);//TIME BETWEEN time_edge_1 AND time_edge_2 
    		
    		count_Ovf=0;
    		/*
			 * count_Ovf: NUMBER OF TIMES AN OVERFLOWS OCCURS
			 * time_pulse_width=(number_of_times_that_happens_OVERFLOW)*(VALUE_MODULE)+time_edge_1-time_edge_2
			 * 
			 * with this formulates, we considered all cases.
			 * 
			 * */
    	}	
    }
    
    
  }
}//main
