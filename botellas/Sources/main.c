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

// Headers files for accessing the MCU registers
#include <hidef.h>
#include "derivative.h"
#include "lcd.h"
#include <string.h>

#define MODULO 30000//VALUE MODULE REGISTER



//
#define B1_LOW 12
#define B1_HIGH 20
unsigned int B2_COUNT=0;

#define B2_LOW 34
#define B2_HIGH 40
unsigned int B1_COUNT=0;

// 47 46 47 50 45 47 50 47 48 48
#define B3_LOW 46
#define B3_HIGH 56
unsigned int B3_COUNT=0;

unsigned int BD_COUNT=0;
int total =0;

char COUNTMSG[50];

char snum[10];



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
	//
	// Datos Display
	// A0,A1,A2,A3
	PTAD=0x00;
	PTADD=0xFF;
	
	// Salida logica motor
	PTFD=0x00;
	PTFDD=0xFF;

	// C6 --> RS
	// C4 --> ENABLE
	PTCD=0x00;
	PTCDD=0xFF;
	
	
	PTBDD=0xFF;
	PTBD=0x00;
	// B0 --> Salida Cuenta
	// B1 --> Salida Dejo de contar
	// B2 --> Salida Alarma defectuoso
	
	//D0 --> Entrada Switche cuenta prendida/apagada 
	//D1--> modo average o absoluto
	//D2--> Activacion del motor Banda (activo o detenido)
	//D4--> Resetear conteo
	//D3--> MODO VISUALIZAR GUIA O CONTEO
	PTDD = 0x00;
	PTDDD = 0x00;
	
	// E2 --> Entrada Sensor
	PTDPE = 0xFF;
	PTED = 0x00;
	PTEDD = 0x00;

	

}
static char *itoa_simple_helper(char *dest, int i) {
  if (i <= -10) {
    dest = itoa_simple_helper(dest, i/10);
  }
  *dest++ = '0' - i%10;
  return dest;
}

char *itoa_simple(char *dest, int i) {
  char *s = dest;
  if (i < 0) {
    *s++ = '-';
  } else {
    i = -i;
  }
  *itoa_simple_helper(s, i) = '\0';
  return dest;
}

void presentation(void){
	  LCD_Clear();
	  
	  LCDWriteCenterMsg(LCD_USE_FIRST_LINE, "SW4: GUIA O CONTEOS",0);
	  LCDWriteCenterMsg(LCD_USE_SECOND_LINE,"SW1: ENC.APA SISTEMA",0);
	  LCDWriteCenterMsg(LCD_USE_THIRD_LINE, "SW3: ENC.APA MOTOR  ",0);
	  LCDWriteCenterMsg(LCD_USE_FOURTH_LINE,"SW8: PORCENTAJES    ",0);
}

void show_count(int i){
	LCD_Clear();
	if (i==0){
	itoa_simple(snum, B1_COUNT);
	strcpy(COUNTMSG, "STANDARD MENOS:");
	LCDWriteMsg(LCD_USE_FIRST_LINE,strcat(COUNTMSG,snum),0);


	itoa_simple(snum, B2_COUNT);
	strcpy(COUNTMSG, "STANDARD      :");
	LCDWriteMsg(LCD_USE_SECOND_LINE,strcat(COUNTMSG,snum),0);

	itoa_simple(snum, B3_COUNT);
	strcpy(COUNTMSG, "STANDARD MAS  :");
	LCDWriteMsg(LCD_USE_THIRD_LINE,strcat(COUNTMSG,snum),0);

	itoa_simple(snum, BD_COUNT);
	strcpy(COUNTMSG, "DEFECTUOSAS   :");
	LCDWriteMsg(LCD_USE_FOURTH_LINE,strcat(COUNTMSG,snum),0);
	}
	else
	{
		total = B1_COUNT+B2_COUNT+B3_COUNT+BD_COUNT;

		itoa_simple(snum, (B1_COUNT*100/total));
		strcpy(COUNTMSG, "STANDARD MENOS%:");
		LCDWriteMsg(LCD_USE_FIRST_LINE,strcat(COUNTMSG,snum),0);


		itoa_simple(snum, (B2_COUNT*100/total));
		strcpy(COUNTMSG, "STANDARD      %:");
		LCDWriteMsg(LCD_USE_SECOND_LINE,strcat(COUNTMSG,snum),0);

		itoa_simple(snum, (B3_COUNT*100/total));
		strcpy(COUNTMSG, "STANDARD MAS  %:");
		LCDWriteMsg(LCD_USE_THIRD_LINE,strcat(COUNTMSG,snum),0);

		itoa_simple(snum, (BD_COUNT*100/total));
		strcpy(COUNTMSG, "DEFECTUOSAS   %:");
		LCDWriteMsg(LCD_USE_FOURTH_LINE,strcat(COUNTMSG,snum),0);
	}

}

//MAIN FUNCTION
void main(void) 
{
  // VARIABLES DECLARATIONS
  char flag=0;//VARIABLE FOR INDICATING A FIRST EDGE OR SECOND EDGE
  //SYSTEM INITIALIZATION
  SOPT1_COPT = 0;
  Port_Init();
  //MCG_Init();
  LCD_Init();
  TIMER1_Init();

  // ENABLE INTERRUPTS 
  EnableInterrupts; 
  Flag_Int_IC=0;


  for(;;) 
  {
	  
	if (PTDD_PTDD4==0){
		B1_COUNT=0;
		B2_COUNT=0;
		B3_COUNT=0;
		BD_COUNT=0;
	}
	
	if (PTDD_PTDD0==1)
	{	
		PTFD_PTFD1=0;
		LCD_Clear();
		LCDWriteCenterMsg(LCD_USE_SECOND_LINE,"SISTEMA INACTIVO",0);
	    if (PTDD_PTDD0 == 0)
	    {
	    	show_count(!PTDD_PTDD1);
	    }

	}
	else
	{
	PTFD_PTFD1=!PTDD_PTDD2;

	  if (PTDD_PTDD3 == 1)
	    {
			presentation();
		}
	  else{show_count(!PTDD_PTDD1);}
		  
		{
			
			__asm WAIT;
    

			if(Flag_Int_IC==1)
			{
				Flag_Int_IC=0;//DON'T FORGET TO TURN OFF THE FLAG
				
				if (flag==0 && PTED_PTED2==0)
				{	//FIRST EDGE
					count_Ovf=0;
					time_edge_1=TPM1C0V;//CAPTURE THE TIME INDICATING A RISING OR FALLING EDGE
					flag=1;//SECOND EDGE
					PTBD_PTBD0=1;//LEDS
					PTBD_PTBD1=0;//LEDS
					time_pulse_width=0;
				}
				else if(flag==1 && PTED_PTED2==1)
				{	//SECOND EDGE
					time_edge_2=TPM1C0V;//CAPTURE THE TIME INDICATING THE OTHERWISE
					flag=0;//FISRT EDGE (NEW PULSE)
					PTBD_PTBD0=0;//LEDS
					PTBD_PTBD1=1;//LEDS
					
					PTBD_PTBD2=0; // Asumimos que no es defectuosa.
					// convert 123 to string [buf]
					if (B1_LOW <= count_Ovf && count_Ovf <= B1_HIGH){
						B1_COUNT++;
						}
					else if (B2_LOW <= count_Ovf && count_Ovf <= B2_HIGH){
						B2_COUNT++;
						}
					else if (B3_LOW <= count_Ovf && count_Ovf <= B3_HIGH){
						B3_COUNT++;
					}
					else {
						BD_COUNT++;
						PTBD_PTBD2=1; // Prendemos señal de alarma.
					}
					
					count_Ovf=0;
				}
			}
		}
    }
  }
}
