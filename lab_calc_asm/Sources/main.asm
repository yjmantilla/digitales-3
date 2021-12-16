; Include derivative-specific definitions
	INCLUDE 'derivative.inc'

; Export Symbols
	XDEF _Startup
	ABSENTRY _Startup

; Constants (ALLCAPS)
MASK_OP:		EQU		%00000011
MASK_BUTTON:	EQU		%00001111
MASK_SIGN:		EQU		%00000100


;*************************************************************************************************
	ORG Z_RAMStart
; Put the origin of the next declared variables at Z RAM START
; These variables will be accesed faster because they are in page zero 

; FAST VARIABLES
; PascalCase
AAscii:				DS.B	4 ; Offset=0
OpAscii:			DS.B	1 ; Offset=4
BAscii:				DS.B	4 ; Offset=5
EqualAscii:			DS.B	1 ; Offset=9
RAscii:				DS.B	4 ; Offset=10
ErrorAscii:			DS.B	1 ; Offset=14

OperandA:			DS.B	1
OperandB:			DS.B	1
Result:  			DS.B	1
Operator:			DS.B	1
SignA:				DS.B	1
SignB:				DS.B	1
SignResult:			DS.B	1
OperandASM:			DS.B	1
OperandBSM:			DS.B	1
ResultSM:  			DS.B	1
Error:				DS.B	1
Sign:				DS.B	1
Hundreds:			DS.B	1
Tens:				DS.B	1
Ones:				DS.B	1
Aux:				DS.B	2 ; auxiliar var
Dummy:				DS.B	1 ; another auxiliar var
plus:				DC.B 	"+"
minus:				DC.B 	"-"
times:				DC.B 	"x"
divided:			DC.B 	"/"
equal:				DC.B 	"="
err:				DC.B 	"E"

;*************************************************************************************************
;
; variable/data section
; SLOW VARS

	ORG RAMStart 
; These variables will be accessed slower, as they are in RAM but not in page zero


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
	CLR SignResult
	CLR Error
	CLR Sign
	CLR Hundreds
	CLR Tens
	CLR Ones
	;CLR AAscii
	;CLR OpAscii
	;CLR BAscii
	;CLR EqualAscii
	;CLR RAscii
	;CLR ErrorAscii
	;CLR Message

	; Below is only for development purposes
	MOV #13,OperandA
	MOV #3,OperandB
	MOV #2,Operator

	; Tests

	; Sum
	; 127+1 -> -128,Error
	; 10 + (-2) --> 8

	; Substraction
	; Need to do more
	; 10 - (-2) --> 12

	; Multiplication
	; 127x127 -> +001,Error
	; 64x-2 --> -128
	; 64x2 --> -128,Error
	; 64x0 --> +0
	; -64x0 --> +0
	; 13x3 --> +39

	; Division
	; 127 / 0 -> 0, Error
	; 8 / 4 -> 2
	; 8 / 3 -> 2
	; 8 / -3 -> -2
	; 3 / 4 -> +0
	; 3 / -4 -> +0
	; -128 / -1 -> -128, Error
	; -128 / 1 -> -128
	; -128 / -2 --> +64
	
	
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

	;Print A Test
	; LDA #0
	; STA Dummy
	; MOV OperandA,Aux
	; JSR print

	; mask Operator to the LSBs
	LDA Operator
	AND #MASK_OP
	BRA choose_action
	
	; Test BIN2DEC
	; LDA OperandA
	; JSR bin_to_dec
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



_sum:
	MOV plus,OpAscii
	MOV #0,Error ; Assume there wont be a problem by default

	; A+B
	LDA OperandB
	ADD OperandA
check_sum:
	TAX ; Temporally save Result (A) in X with TAX so that CCR does not change

	; Check no-overflow
	TPA
	; We must now check bit 7 (V), if A is negative means it is ON
	ADD #0
	BPL no_problem_sum
	MOV #1,Error ; There was overflow
no_problem_sum:
	TXA
	JMP show_result
_sub:
	MOV minus,OpAscii
	; Does A - B by default
	; So we need to negate B
	LDA OperandA
	SUB OperandB
	BRA check_sum ; Continue with algebraic Sum with signed integers
_mul:
	; Notice MUL is only capable of doing unsigned multiplication
	JSR get_A_B_sign_magnitude
	MOV #0,Error ; Assume there wont be a problem by default
	MOV times,OpAscii
	LDA OperandASM
	LDX OperandBSM
	MUL
	STA ResultSM
	; If (X:A[7]) != 0x0000:0b0, then result of the multiplication 
	; cannot be represented with 8bit signed integer
	; somehow we need to check for this and feedback to the user
	; truncante to A[0:7) ?
	TXA ; If X is 0, we dont suspect there is a problem yet
	ADD #0 ; Force CCR update (TXA does not do it)
	BEQ check_sign
	MOV #1,Error
check_sign:
	; Now check the MSB of A, needs to be zero for no problem, that is, needs to be a "positive"
	; NOTE: This logic misses the correct case of the multiplication being -128
	; we could just check that the result is not the special case of -128
	LDA ResultSM
	BPL get_muldiv_sign
	MOV #1,Error ; By this point u128 will give warning
get_muldiv_sign:
	; Assumes ResultSM has the unsigned result

	; Get sign of the multiplication/division with exclusive or
	; Assumes SignA SignB Set
	; Asummes output in Result
	LDA SignA
	AND #MASK_SIGN
	STA Aux
	LDA SignB ;
	AND #MASK_SIGN
	EOR Aux ; If Z=0-> Different Signs, If Z=1-> Same Signs
	MOV #43,Aux
	ADD #0
	BEQ pos_sign
	MOV #45,Aux ; Negative
pos_sign:
	LDA ResultSM
	STA Result ;

	LDA Aux
	STA SignResult
	ADD #0
	BRCLR 2 , SignResult , muldiv_is_pos
	LDA ResultSM
	NEGA
	STA Result ; notice NEG(-128)-->(-128)
muldiv_is_pos:
	; Handle 128 Case
	; Check if Result is u128 and ResultSign is Negative (representable) 
	; Here we need to overwite de Error flagged on before
	LDA Result
	ADD #-128 ; u128 + (-)128=0
	BNE not_128
	; Confirmed 128
	; Check Sign, Overwite Error Only if ResultSign = 1 (negative)
	LDA SignResult
	CMP #43
	BEQ not_128 ; If Sign is Positive (A=0)--> +128--> Unrepresentable --> Keep the Error of Before
	; Here, sign is negative.
	; Overwrite Error
	MOV #0,Error
not_128:
	LDA Result
	JMP show_result
_div:
	; Notice DIV is only capable of doing unsigned division
	; A / B
	; Divide, A <-- (H:A) / (X); H <- Remainder
	MOV #0,Error ; Assume there wont be a problem by default
	MOV divided,OpAscii
	JSR get_A_B_sign_magnitude

	; Check B is not 0
	LDA OperandBSM
	BEQ division_by_zero

	; What happens if A < B, does div handles it gracefully?
	LDA OperandASM
	LDHX #0 ; Clear H
	LDX OperandBSM
	DIV
	STA ResultSM
	JMP check_sign
division_by_zero:
	AND #0 ; Clear A
	MOV #1,Error ; Have special led for zero division?
	JMP show_result

get_A_B_sign_magnitude:
	; Subroutine
	; Modifies OperandASM,SignA,OperandBSM,SignB,CCR
	; puts the Signs of A,B in SignA,SignB respectively
	; and makes OperandASM,OperandBSM their absolute value
	LDA OperandA
	JSR get_sign_magnitude
	STA OperandASM
	STX SignA
	LDA OperandB
	JSR get_sign_magnitude
	STA OperandBSM
	STX SignB
	RTS

get_sign_magnitude:
	; Assumes A is input in 2 complement
	; Saves sign at X
	; Saves uint at A
	ADD #0
	BPL is_positive
	BMI is_negative
is_positive:
	LDX #43
	; A is already at correct representaion
	RTS
is_negative:
	NEGA
	LDX #45
	RTS

bin_to_dec:
	; Assume input is at A
	; Modifies HX
	JSR get_sign_magnitude
	; X=SIGN , A=uint
	; Check B is not 0
	STX Sign
	LDHX #0 ; Clear H
	LDX  #100
	DIV ; A--> Division 9, H--> Residuo-->21
	ADD #48
	STA Hundreds
	STHX Aux
	LDA Aux
	LDHX #0
	LDX #10
	DIV
	ADD #48
	STA Tens
	STHX Aux
	LDA Aux
	ADD #48
	STA Ones
	RTS

print:
	;Assume 
	;	num to print at Aux
	;	offset from RamStart at Dummy
	LDA Aux
	JSR bin_to_dec
	
	; Define origin of print
	LDHX #0
	LDA Dummy
	TAX

	LDA Sign
	STA AAscii,X

	LDA Hundreds
	INCX
	STA AAscii,X
	
	LDA Tens
	INCX
	STA AAscii,X

	LDA Ones
	INCX
	STA AAscii,X

	RTS

show_result:
	; Assume No error
	MOV #0,ErrorAscii

	STA Result
	LDA OperandA
	MOV #0,Dummy
	STA Aux
	JSR print

	; Operation Already Printed in each op's function

	LDA OperandB
	MOV #5,Dummy
	STA Aux
	JSR print

	MOV equal,EqualAscii

	LDA Result
	MOV #10,Dummy
	STA Aux
	JSR print

	LDA Error
	CMP #1
	BNE no_error
	MOV err,ErrorAscii
no_error:
	nop
	JMP capture_wait


	ORG $FFFE
	DC.W _Startup ; Reset
