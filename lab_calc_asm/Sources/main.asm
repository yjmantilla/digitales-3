; Include derivative-specific definitions
	INCLUDE 'derivative.inc'

;
; export symbols
;
	XDEF _Startup
	ABSENTRY _Startup

;*************************************************************************************************

	ORG Z_RAMStart ; Put the origin of the next declared variables at Z RAM START
; These variables will be accesed faster because they are in page zero 

;**********		VARIABLES		**********

OperantA:			DS.B	1
OperantB:			DS.B	1
Result:  			DS.B	1
Operant:			DS.B	1

;*************************************************************************************************
;
; variable/data section
;
	ORG RAMStart 
	; These variables will be accessed slower, as they are in RAM but not in page zero

ExampleVar: DS.B   1

;
; code section

;*********************************************************************************************
	ORG ROMStart ; ROM memory will hold the code. Notice it cannot be modified in runtime.

_Startup:
	; ********** CONFIGURATIONS WATCHDOG AND STACK **********
	; Power off WatchDOG
	LDA   #$20
	STA   SOPT1
	
	;Define stack start 
	LDHX #RAMEnd+1 ; initialize the stack pointer at RAM END
	TXS ;sp<-(H:X)-1
	
	;************* Clear variables *********************************
	CLRA
	CLRX
	CLRH
	CLR OperantA
	CLR OperantB
	
	
	
; ******** Initializing In ports ******************************

	MOV #$FF, PTAD	; 
	MOV #$00, PTADD ; Puertos de los swiches
	
	MOV #$00, PTBD
	MOV #$00, PTBDD ; Puerto del bus
	
;***************************************************************
; Ingresa el primer operando	
			
esperar:	BRCLR 0, PTAD, Debounce	;Allow input operantA 
     		BRCLR 1, PTAD, Debounce ;Allow input operantB
     		;BRCLR 2, PTAD, debounce ;Allow input operant
     		;BRCLR 3, PTAD, debounce ;Allow input START ( EJECUTE OPERATION )
; here:		BRA here
     		;BRA esperar
	
        		
;Final:		BRA Final ; End of program body

;************	SUBRUTINES OR FUNCTIONS		**********************************

Debounce:  	LDHX #5000 

Delay:		AIX #-1 ; LE RESTO 1  A H:X
			CPHX #0 ; compara el valor de H:X con 0
			BNE Delay; BRANCH NO EQUAL, si (H:X)=/0 
			BRCLR 0, PTAD, CAPTURAR_1
			BRCLR 1, PTAD, siga
			;BRCLR 2, PTAD, OPERANDO
			;BRCLR 3, PTAD, INICIO
			BRA esperar
			
CAPTURAR_1:     MOV PTBD,OperantA
			BRA esperar
						
 

siga: 		MOV PTBD,OperantB
			
			BRA  esperar 
			nop  
				
            ORG	$FFFE

						
			DC.W  _Startup			; Reset
