//This program sends and receives data through serial port 
//using polling technique for both transmit and receiving data.
//Adapted by : eaduque 20-07-14.
//**************************************************************/

/* CONEXIONES:
 * 
 *  ______________________
 * |              PTC3(Tx)|--> USB-IN (o MAX-IN)   
 * |         MCU          |
 * |              PTC5(Rx)|<-- USB-OUT (o MAX-OUT)
 * |______________________|
 *       
 * */


#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */

#define CR   0x0D // Posicion del Carrier Return

// Prototipos de funciones
void MCG_Init(void);
void SCI_Init(void);
void SCI_PutChar(byte);
void SCI_PutMsg(const byte *);
byte SCI_GetChar(void);

const byte Msg[] = "Bienvenido al curso de ElectrOnica Digital III\n\n\r";
byte response = 0;
volatile byte tecla = 0;
volatile byte flag_rx = 0;

interrupt 23 void READ_ISR(void){//INTERRUPT ENABLE
	(void) SCI2S1;//If hardware interrupts are used, SCIxS1 must be read in the interrupt service routine (ISR). 
	tecla = SCI2D;
	flag_rx = 1;
	//response = SCI_GetChar();
	//SCI_PutChar(response);

}

void main(void) {
   	SOPT1 = 0x20;
	MCG_Init(); // Configurar Relog
	SCI_Init(); // Configurar Serial Communication
	
	PTBDD_PTBDD7 = 1;
	PTBD_PTBD7 = 0;
	
	SCI_PutChar(CR);
	EnableInterrupts;

	for(;;) {
	
		SCI_PutMsg("InformaciOn: No. estudiantes, Tipo de curso, Nivel. Digite (n/t/s): ");
		asm WAIT;
		if (flag_rx==1){
		flag_rx = 0;
		
		switch(tecla) {
		  case 'n': 
			SCI_PutMsg("\n\r\n\rTiene 20 estudiantes!!!n \n\r\n\r");
			break;
		  case 't':
			SCI_PutMsg("\n\r\n\rCurso regular obligatorio!!! \n\r\n\r");
			break;
		  case 's':
			SCI_PutMsg("\n\r\n\rCurso de octavo semestre!!! \n\r\n\r");
			break;
          default:
			SCI_PutMsg("\n\r\n\rPregunta incorrecta.Por favor repita pregunta !!\n\r\n\r");
			break;
		}
		}
		
	}
}
	
void SCI_Init(void) {
  SCI2BDL = 78; // SCI Baud rate = BusClock/16*[SBR12:SBR0] ---> 12Mhz/(16*78) = 9600 bps
  SCI2C2 = (SCI2C2_TE_MASK | SCI2C2_RIE_MASK | SCI2C2_RE_MASK); //Masks defined by mcs908jm60.h
  /* By default M=0 inside(SCIxC1) --> Normal -- Start + 8bits (LSB first) + Stop
  PE Parity Enable 0 --> No hardware parity generation or checking
  PT  Parity Type 0 Even parity */
}

void SCI_PutChar(byte Data) {
  while(SCI2S1_TDRE == 0);
  SCI2D = Data;
}

byte SCI_GetChar(void) {
  while(SCI2S1_RDRF == 0);
  return SCI2D;// The RDRF flag is cleared at end of execution. Why?
}

void SCI_PutMsg(const byte * Data) {
  while(*Data != '\0') {
	SCI_PutChar(*Data++);
  }
}

// Moving from FEI to PEE Mode: External Crystal = 12 MHz,Bus Frequency = 12 MHz
void MCG_Init(void) {
	MCGC2 = MCGC2_RANGE_MASK | MCGC2_HGO_MASK | MCGC2_EREFS_MASK | MCGC2_ERCLKEN_MASK;                                      
	while(MCGSC_OSCINIT != 1){};
	MCGC1 = MCGC1_CLKS1_MASK | MCGC1_RDIV_MASK;
	while (MCGSC_IREFST != 0){};
	while (MCGSC_CLKST != 0b10){};
	MCGC2 |= MCGC2_LP_MASK;  
	MCGC1_RDIV = 0b011;
	MCGC3 = MCGC3_PLLS_MASK | MCGC3_VDIV2_MASK;                                      
	while (MCGSC_PLLST != 1){};	
	MCGC2_LP = 0;                                 
	while (MCGSC_LOCK == 0){};                  
	MCGC1_CLKS = 0b00;
	while(MCGSC_CLKST != 0b11){};	
}
