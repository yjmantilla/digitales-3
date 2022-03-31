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
#include <time.h>

#define CR   0x0D // Posicion del Carrier Return
//#define BOBINAA PTED_PTED3
//#define BOBINAB PTED_PTED1
//#define BOBINAC PTFD_PTFD6
//#define BOBINAD PTFD_PTFD7
// Prototipos de funciones
void MCG_Init(void);
void SCI_Init(void);
void SCI_PutChar(byte);
void SCI_PutMsg(const byte *);
byte SCI_GetChar(void);

const byte Msg[] = "Bienvenido al curso de ElectrOnica Digital III\n\n\r";
volatile byte MsgRec[91] = ""; //([1]+,+[4]+,+[1]+;)*10 + $
byte response = 0;
volatile byte tecla = 0;
volatile byte flag_rx = 0;
volatile int counter = 0;
volatile byte flag_movement = 0;
volatile int echo_counter = 0;
volatile int flag_salto =0;
volatile int flag_end =0;
volatile int directions[10]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
volatile int degrees[10]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
volatile int pauses[10]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
volatile int which_part = 0;
volatile byte currChar=0;
volatile byte currStr[4]="****";
volatile int currVal = 0;
volatile byte strIndex = 0;
volatile int currMove = 0;
volatile int index = 0;
volatile byte flag_ready=0;
volatile int odd=0;

// Bobinas
volatile byte ins=0, ind=0, ba=0, bb=0, bc=0, bd=0;
int pasos=0;
int sentido=0;
int frec=50;
int op=0;
int deg=0;
int t_paso=1;
float prec=64/(2*5.625); 
float ftem=0;
unsigned long pause=0;//1000/200;
int a =0;
unsigned long mod = 0;
interrupt 23 void READ_ISR(void){//INTERRUPT ENABLE
	(void) SCI2S1;//If hardware interrupts are used, SCIxS1 must be read in the interrupt service routine (ISR). 
	tecla = SCI2D;
	flag_end = 0;
	flag_movement = 0;
	flag_salto=0;
	if (tecla=='$'){
		flag_end = 1;
	}
	if (tecla == ';'){
		flag_movement = 1;
		
	}
	if (tecla == '\n'){
		flag_salto = 1;
	}
	flag_rx = 1;
	//response = SCI_GetChar();
	//SCI_PutChar(response);

}

void delay(int milli_seconds)
{
	int j=0;
	for (j=0;j<milli_seconds;j=j+1){
		1+1;
	}
    // Converting time into milli_seconds  
    // Storing start time
//    clock_t start_time = clock();
  
    // looping till required time is not achieved
//    while (clock() < start_time + milli_seconds)
//        ;
}

int atoi(volatile char* str){
    int num = 0;
    int i = 0;
    while (str[i] && (str[i] >= '0' && str[i] <= '9')){
        num = num * 10 + (str[i] - '0');
        i++;
    }
    return num;
}

void mediopaso(int n){
  int i=0;
  ba=1; bb=0; bc=0; bd=0;
  for(i=1;i<=n;i++){
    if(sentido==0){
      if((ba==1 && bd==0) && bb==0){
         PTFD_PTFD7=0;       
         PTED_PTED3=1;
         delay(pause);
         bb=1;
      }else if(ba==1 && bb==1){
        PTED_PTED1=1;
        delay(pause);
        ba=0;
      }else if(bb==1 && bc==0){
         PTED_PTED3=0;
         delay(pause);
         bc=1;
      }else if(bb==1 && bc==1){
         PTFD_PTFD6=1;
         delay(pause);
         bb=0;
      }else if(bc==1 && bd==0){
        PTED_PTED1=0;
        delay(pause);
        bd=1;
      }else if(bc==1 && bd==1){
        PTFD_PTFD7=1;
        delay(pause);
        bc=0;
      }else if(bd==1 && ba==0){
        PTFD_PTFD6=0;
        delay(pause);
        ba=1;
      }else if(bd==1 && ba==1){
         PTED_PTED3=1;
         delay(pause);
         bd=0;
      }
    }else{
      if(ba==1 && bd==0 && bb==0){
         PTFD_PTFD7=0;
         PTED_PTED1=0;       
         PTED_PTED3=1;
         delay(pause);
         bd=1;
      }else if(ba==1 && bb==1){
        PTED_PTED3=1;
        delay(pause);
        bb=0;
      }else if(bb==1 && bc==0){
         PTFD_PTFD6=0;
         delay(pause);
         ba=1;
      }else if(bb==1 && bc==1){
         PTED_PTED1=1;
         delay(pause);
         bc=0;
      }else if(bc==1 && bd==0){
        PTFD_PTFD7=0;
        delay(pause);
        bb=1;
      }else if(bc==1 && bd==1){
        PTFD_PTFD6=1;
        delay(pause);
        bd=0;
      }else if(bd==1 && ba==0){
        PTED_PTED3=0;
        delay(pause);
        bc=1;
      }else if(bd==1 && ba==1){
         PTFD_PTFD7=1;
         delay(pause);
         ba=0;
      }
    }    
  }
}

void main(void) {
   	SOPT1 = 0x20;
	MCG_Init(); // Configurar Relog
	SCI_Init(); // Configurar Serial Communication
	PTBDD_PTBDD7 = 1;
	PTBD_PTBD7 = 0;
	// Salida logica motor
	PTFD=0x00;
	PTFDD=0xFF;	
	// Salida logica motor
	PTED=0x00;
	PTEDD=0xFF;
	
	SCI_PutChar(CR);
	EnableInterrupts;
	SCI_PutMsg(Msg);
	for(;;) {
		asm WAIT;
		if (flag_rx==1){
		flag_rx = 0;
		if (flag_salto==0){
		MsgRec[counter] = tecla;
		
		if (flag_end){
			counter =0; //For new set of movements?
			flag_end =0;
			index = 0;
			currChar = MsgRec[index];
			currMove=0;
			while(currChar!='$')
			{
				if (currChar >= '0' && currChar <= '9')
				{
					currStr[strIndex]=currChar;
					strIndex+=1;
				}
				else if (currChar == ',')
				{
					currStr[strIndex]='\0';
					strIndex=0;
					currVal = atoi(currStr);
					
					switch (which_part)
					{
					case 0:
						directions[currMove] = currVal;
						break;
					case 1:
						degrees[currMove] = currVal;
						break;
					case 2:
						pauses[currMove] = currVal;
						break;
					default:
						break;
					}
					
					which_part+=1;
					currStr[1] = '*';
					
				}
				else if (currChar==';'){
					which_part=0;
					currMove +=1;
				}
				index+=1;
				currChar = MsgRec[index];
				
				
			}
			if (currChar=='$'){
				flag_ready=1;
				a = 0;

				for(a = 0; a < 10; a = a + 1){
					pasos=(int) degrees[a]*6; // reconfigurar el codewarrior para usar puntoflotante
					sentido = directions[a];
					pause = 2000;//pauses[a];
					mediopaso(2*pasos);
				}
			}
			
		}
		counter = counter +1;
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

void SCI_PutMsg(byte * Data) {
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


