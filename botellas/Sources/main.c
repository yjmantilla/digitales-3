// Headers files for accessing the MCU registers
#include <hidef.h> //interrupciones enable interrupt
#include "derivative.h"

//Variables for counting and indicating IRQ event.
volatile unsigned char Flag_Int;//variable for indicating to main program that an IRQ event has ocurred .... GLOBAL

unsigned char Contador = 0; // uchar es estandar?, uint no tanto

// Usar BDM y Full chip,,,,,, cyclone y usb no.
// Values sent to 7 segment display according to counter value
const unsigned char VectorDisplay[] =
{
 0x3F,0x06, 0x5B,0x4F,0x66,  //segment On with 1 logic
 0x6D,0x7D,0x07,0x7F,0x6F,
 0x77,0x7C,0x39,0x5E,0x79,
 0xF1
 /*0xC0, 0xF9, 0xA4,0xB0, 0x99,0x92,  //segment On with 0 logic
 0x82,0xF8,0x80,0x90,0x08,0x03,
 0x46,0x21,0x06,0x0e*/
};
// 11000000
// IRQ module function initialization
void IRQ_Init(void)
{
// pag 72 manual
    IRQSC_IRQPDD = 0; // Data direction??? Siempre arranca en 0.
    // Tiene que ver con que si tiene un pull up (0). Generalmente se deja en 0.
    IRQSC_IRQEDG = 0; // EDGE 0--> SubidBajada
    IRQSC_IRQMOD = 0; // 0--> Flanco,  1---> Flanco y nivel . IRQ Detection Mode, Trabaja con IRQ EDGE
    IRQSC_IRQPE = 1;// Pin Enable. Tiene que estar en 1 para poder interrupciones
    IRQSC_IRQACK = 1; // acknowledge //prevent false triggering.
    IRQSC_IRQIE = 1; // Interrupt Enable,mascara que trabaja con IRQF IRQ Flag --> Pasan a una and
   
    // IRQF lo maneja el sistema. -->Bandera que dice cuando una interrupcion se solicita, es un status.
    // Pedido de interrupcion puede ser 4 posibles:
    // - Flanco de bajada
    //  - Flanco de subida
    //  - Flanco y nivel...
    // - de subida
    // - de bajada
   
    // El reset lleva todos los bits de IRQSC a 0
    // para esto necesitamos el init
   
    // Boton lo trabaja tierra.
}

// Port B function initialization.
void Port_Init(void)
{
  PTCD = VectorDisplay[0];
  PTED = 0x00;
  PTDD = 0x00;
  PTCDD = 0xFF;
  PTDDD = 0x00;
  //PTDPE = 0xFF;
  PTEDD = 0xFF;
}

// IRQ Interrupt Service Routine

interrupt 2 void ISR_IRQ(void)
{
  IRQSC_IRQACK = 1; //Power off IRQF // Para decir que estamos atendiendo la introduccion, quitarla ,instruccion solo valida con flanco basico
  Flag_Int = 1; //for comunicating to main that IRQ event has ocurred
 
  // 2 es la prioridad
 }

// Main function
void main(void)
{
  // First. Variables declarations
//unsigned int i; //If Delay is required
 
  // System Initialization
  SOPT1_COPT = 0;
  IRQ_Init();
  Port_Init();
 
  // Las instrucciones de C las corre bien, pero las de assembler a veces da problemas.
  // Poner PeMIcro --> IRQ --> IRQ =0

  // Enable Interrupts
  EnableInterrupts; // Inside a header file is :  Define EnableInterrupts  __asm CLI ;
 
  for(;;)
  {
    // Bajo consumo
 // Toca simular el flanco 1-->0
    //__asm WAIT;
	  PTED_PTED1 = !PTDD_PTDD1;

    /* for (i=0; i<32000;i++);
    debounce software is not recommended.*/
    /*
    if (Flag_Int == 1)   //
    {
   
   
      // clear varible that indicates that IRQ event ocurred
      Flag_Int = 0;
      // Increment counter and displays count
      Contador++;
      Contador &= 0x0F; // No usamos los bits completos, algunos no nos ineteresan
      PTCD  =  VectorDisplay[Contador];
    }*/
  }// end of for
}
