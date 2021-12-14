; Include derivative-specific definitions
	INCLUDE 'derivative.inc'

; Export Symbols
	XDEF _Startup
	ABSENTRY _Startup

; Constants (ALLCAPS)
MASK_OP: EQU %00000011
MASK_BUTTON: EQU %00001111

;*************************************************************************************************
	ORG Z_RAMStart
; Put the origin of the next declared variables at Z RAM START
; These variables will be accesed faster because they are in page zero 

; FAST VARIABLES
; PascalCase
OperandA:			DS.B	1
OperandB:			DS.B	1
Result:  			DS.B	1
Operator:			DS.B	1
SignA:				DS.B	1
SignB:				DS.B	1
Warning:			DS.B	1
;*************************************************************************************************
;
; variable/data section
; SLOW VARS

	ORG RAMStart 
; These variables will be accessed slower, as they are in RAM but not in page zero

ExampleVar: DS.B   1


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
	CLR OperandA
	CLR OperandB
	CLR Operator
	CLR Result
	CLR SignA
	CLR SignB
	CLR Warning
	
	; Below is only for development purposes
	MOV #3,OperandA
	MOV #-4,OperandB
	MOV #3,Operator

	; Tests

	; Sum
	; 127+1 -> Warning

	; Substraction
	; Need to do more

	; Multiplication
	; 127x127 -> Warning
	; Need to do more
	; Note -128 as result should be valid but in current implementation it is not

	; Division
	; 127 / 0 -> Warning
	; 8 / 4 -> 2
	; 8 / 3 -> 2 (Rem 2)
	; 8 / -3 -> -2
	; 3 / 4 -> 0
	; 3 / -4 -> 0
	
	
; ******** Initializing In ports ******************************
	; It is recommended to load the data of the ports before configuring them
	; PTA will be for buttons	
	; PTA : [X,X,X,X,Start,Op,B,A]

	MOV #$FF, PTAD  ; safe load
	MOV #$00, PTADD ; PTA as INPUT (all lines)
	; Notice that we wont need bit 7 and 6 of PTA, so no problem...
	
	; PTB will be for data
	; PTB has no deactivated bits
	MOV #$00, PTBD  ; safe load
	MOV #$00, PTDDD ; PTB as INPUT (all lines)

	; PTC will be for identifying the operation
	MOV #$00, PTCD  ; safe load
	MOV #$FC, PTCDD ; PTC as INPUT (all lines)
	
	
	;00 --> Sum
	;01 --> Substract
	;10 --> Multiply
	;11 --> Division


; Maybe add a LED to indicate if there was trouble doing an operation
; Add leds to visualize output result
;***************************************************************
; Ingresa el primer operando

capture_wait:
	; We will wait for button signals indicating what data is ready

	; Check if any of the buttons is pressed
	LDA PTAD
	AND #MASK_BUTTON
	; If Z=1, no button pressed
	ADD #1 ; Forces Z=0, For development purposes so that it does something
	BEQ capture_wait
	;BRCLR 0, PTAD, debounce ;Allow input OperandA 
	;BRCLR 1, PTAD, debounce ;Allow input OperandB
	;BRCLR 2, PTAD, debounce ;Allow input Operator
	;BRCLR 3, PTAD, debounce ;Allow input START ( EXECUTE OPERATION )
	;JMP debounce ; If real buttons
	JMP start ; For development purposes, input stuff on the variable directly
;here:
	;BRA here
	;BRA esperar

;final:
	;BRA Final
	;End of program body

;************	SUBRUTINES OR FUNCTIONS		**********************************
; It matters if the subrutine is far from whoever calls it, i had to put start nearer capture_wait
; or you may use JMP?

debounce:
	LDHX #5000

delay:
	AIX #-1 ; Substract one
	CPHX #0 ; Check if we are already at zero
	BNE delay; Not Zero--> not ready, loop back
	BRA choose_capture ; We waited enough... go on with the capture

choose_capture:
	BRCLR 0, PTAD, capture_a
	BRCLR 1, PTAD, capture_b
	BRCLR 2, PTAD, capture_op
	BRCLR 3, PTAD, start
	JMP capture_wait

capture_a:
	MOV PTBD,OperandA
	JMP capture_wait

capture_b:
	MOV PTBD,OperandB
	JMP capture_wait

capture_op:
	MOV PTCD,Operator
	JMP capture_wait

start:
	; mask Operator to the LSBs
	LDA Operator
	AND #MASK_OP
	BRA choose_action

choose_action:
	; Context: at this point acumulator is the operation
	; Logic: Decrement Accumulator to know which op it is
	ADD #0 ; Just so that we assure CCRs are updated
	BEQ _sum ; If A=0 => A-0=0
	DECA
	BEQ _sub ; If A=1 => A-1=0
	DECA
	BEQ _mul ; If A=2 => A-2=0
	DECA
	BEQ _div ; If A=3 => A-3=0
	BRA capture_wait

show_result:
	STA Result
	BRA capture_wait

_sum:
	MOV #0,Warning ; Assume there wont be a problem by default
	; A+B
	LDA OperandB
	ADD OperandA
	TAX ; Temporally save Result (A) in X with TAX so that CCR does not change

	; Check no-overflow
	TPA
	; We must now check bit 7 (V), if A is negative means it is ON
	ADD #0
	BPL no_problem_sum
	MOV #1,Warning ; There was overflow
no_problem_sum:
	TXA
	JMP show_result
_sub:
	; Does A - B by default
	; So we need to negate B
	NEG OperandB
	BRA _sum ; Continue with algebraic Sum with signed integers
_mul:
	; Notice MUL is only capable of doing unsigned multiplication
	JSR _get_sign_magnitude
	MOV #0,Warning ; Assume there wont be a problem by default
	LDA OperandA
	LDX OperandB
	MUL
	STA Result
	; If (X:A[7]) != 0x0000:0b0, then result of the multiplication 
	; cannot be represented with 8bit signed integer
	; somehow we need to check for this and feedback to the user
	; truncante to A[0:7) ?
	TXA ; If X is 0, we dont suspect there is a problem yet
	ADD #0 ; Force CCR update (TXA does not do it)
	BEQ no_mul_problem1
	MOV #1,Warning
no_mul_problem1:
	; Now check the MSB of A, needs to be zero for no problem, that is, needs to be a "positive"
	; NOTE: This logic misses the correct case of the multiplication being -128
	; we could just check that the result is not the special case of -128
	LDA Result
	BPL no_mul_problem2
	MOV #1,Warning
no_mul_problem2:
	;Everything seems fine
get_muldiv_sign:
	; Get sign of the multiplication/division with exclusive or
	; Assumes SignA SignB Set
	; Asummes output in Result
	LDA SignA
	EOR SignB ; If Z=0-> Different Signs, If Z=1-> Same Signs
	BEQ muldiv_is_pos
	NEG Result
muldiv_is_pos:
	LDA Result
	JMP show_result
_div:
	; Notice DIV is only capable of doing unsigned division
	; A / B
	; Divide, A <-- (H:A) /�(X); H <- Remainder
	MOV #0,Warning ; Assume there wont be a problem by default

	JSR _get_sign_magnitude

	; Check B is not 0
	LDA OperandB
	BEQ division_by_zero

	; What happens if A < B, does div handles it gracefully?
	LDA OperandA
	LDHX #0 ; Clear H
	LDX OperandB
	DIV
	STA Result
	JMP get_muldiv_sign
division_by_zero:
	AND #0 ; Clear A
	MOV #1,Warning ; Have special led for zero division?
	JMP show_result


_get_sign_magnitude:
	; Subroutine
	; Modifies A,SignA,SignB,CCR
	; puts the Signs of A,B in SignA,SignB respectively
	; and makes OperandA,OperandB their absolute value

	; Assume both positive by default
	; Only changed if proven otherwise
	MOV #0,SignA
	MOV #0,SignA

	LDA OperandA
	BPL _a_is_positive
	NEG OperandA
	MOV #1,SignA
_a_is_positive:
	LDA OperandB
	BPL _b_is_positive
	NEG OperandB
	MOV #1,SignB
_b_is_positive:
	RTS

	ORG $FFFE
	DC.W _Startup ; Reset
