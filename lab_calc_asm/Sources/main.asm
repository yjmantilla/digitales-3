; Include derivative-specific definitions
	INCLUDE 'derivative.inc'

; Export Symbols
	XDEF _Startup
	ABSENTRY _Startup

;**************************************************************************************************
; Constants (ALLCAPS)
;**************************************************************************************************

; Masks, good to obtain relevant bits quickly
MASK_OP:		EQU		%00000011
MASK_BUTTON:	EQU		%00001111
MASK_SIGN:		EQU		%00000100

; Operations Binary Codes
SUM_CODE:		EQU		%00
SUB_CODE:		EQU		%01
MUL_CODE:		EQU		%10
DIV_CODE:		EQU		%11

; ASCII Math symbols to be visualized by the user
PLUS:			EQU		"+"
MINUS:			EQU		"-"
TIMES:			EQU		"x"
DIVIDED:		EQU		"/"
EQUAL:			EQU		"="
ERR:			EQU		"E" ; The ASCII representation of the error flag
NOERR:			EQU		" " ; No error ASCII
EMPTY:			EQU		" " ; Just empty space
ASCII_OFFSET	EQU		48
; Ascii Offsets
; See MESSAGE TO USER for context of this
OFFSET_AAscii:			EQU		0  ; Offset=0
OFFSET_OpAscii:			EQU		4  ; Offset=4
OFFSET_BAscii:			EQU		5  ; Offset=5
OFFSET_EqualAscii:		EQU		9  ; Offset=9
OFFSET_RAscii:			EQU		10 ; Offset=10
OFFSET_ErrorAscii:		EQU		14 ; Offset=14

; Constant for bypassing input through
; Switches and buttons
DEV_MODE		EQU		1

;**************************************************************************************************
; PAGE ZERO RAM
;**************************************************************************************************

	ORG Z_RAMStart
; Put the origin of the next declared variables at Z RAM START
; These variables will be accesed faster because they are in page zero 
; Use PascalCase

; Try to not put too many variables here as to not collide with the stack

; 2's Complement Representation
OperandA:			DS.B	1 ; Operand A in 2's complement
OperandB:			DS.B	1 ; Operand B in 2's complement
Result:  			DS.B	1 ; Result in 2's complement
Operator:			DS.B	1 ; Binary code of the operator

; Wrong Result Indicator
; This will turn ON in any situation the result is not representable
Error:				DS.B	1

; Auxiliar Variables for making stuff easier to do
Aux:				DS.B	2 ; At some point we will store HX so we need 2 positions
Dummy:				DS.B	1

;**************************************************************************************************
; RAM outside of Page Zero
;**************************************************************************************************

; These variables will be accessed slower, as they are in RAM but not in page zero
	ORG RAMStart

; Sign Magnitude Representation
; Useful for multiplication, division and displaying in decimal
SignA:					DS.B	1 ; Sign of A 0:+,1:-
SignB:					DS.B	1 ; Sign of B 0:+,1:-
SignResult:				DS.B	1 ; Sign of Result 0:+,1:-
OperandAABS:			DS.B	1 ; Abs Value of A
OperandBABS:			DS.B	1 ; Abs Value of B
ResultABS:  			DS.B	1 ; Abs Value of Result

; ASCII Representation for the bin_to_dec routine
; These variables are placeholders whose data is then saved in another variable
; DECIMAL with 3 positions and sign: +000
Sign:				DS.B	1
Hundreds:			DS.B	1
Tens:				DS.B	1
Ones:				DS.B	1

;**************************************************************************************************
; MESSAGE TO USER
;**************************************************************************************************
	ORG $0130
AAscii:				DS.B	4 ; Offset=0
OpAscii:			DS.B	1 ; Offset=4
BAscii:				DS.B	4 ; Offset=5
EqualAscii:			DS.B	1 ; Offset=9
RAscii:				DS.B	4 ; Offset=10
ErrorAscii:			DS.B	1 ; Offset=14

; These variables save the ASCII representation of each part
; of the intended operation of the user
; Thus the user can see whats happening in the memory
; In a human-readable way
; These variables are contiguous so that the message is "one sentence"

; We keep count of the offset so that we can
; access each one of these variables
; using only one parameter
; thus the first variable (AAscii) plays as a pointer to the others
; when given the appropiate offset

; With this the same routine can print to any of this variables
; Only given the offset and starting from AAscii
; See "print" subroutine


;**************************************************************************************************
; ROM Memory
;**************************************************************************************************

	ORG ROMStart ; ROM memory will hold the code. Notice it cannot be modified in runtime.

_Startup:
	;********************************************
	; CONFIGURATIONS WATCHDOG AND STACK
	;********************************************
	; Power off WatchDOG
	LDA   #$20
	STA   SOPT1
	
	;Define stack start 
	LDHX #RAMEnd+1 ; initialize the stack pointer at RAM END
	TXS ;sp<-(H:X)-1
	
	;********************************************
	; CLEAR VARIABLES
	;********************************************
	CLRA
	CLRX
	CLRH

	CLR OperandA
	CLR OperandB
	CLR Operator
	CLR Result
	CLR Error
	CLR Aux
	CLR Dummy

	; Initializing outside page zero
	LDA #0
	STA SignA
	STA SignB
	STA SignResult
	STA Sign
	STA Hundreds
	STA Tens
	STA Ones

	LDA #EMPTY
	STA AAscii
	STA OpAscii
	STA BAscii
	STA EqualAscii
	STA RAscii
	STA ErrorAscii


	;********************************************
	; DEV ONLY CODE
	;********************************************

	; Below is only for development purposes
	; This should be modified by the user interacting
	; with the switches and buttons

	LDA #DEV_MODE
	CMP #1
	BNE init_ports
	MOV #13,OperandA
	MOV #3,OperandB
	MOV #2,Operator

	;********************************************
	; Testing the calculator
	;********************************************
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

	;********************************************
	; Initializing In ports
	;********************************************
	; It is recommended to load the data of the ports before configuring them
	; PTA will be for buttons	
	; PTA : [X,X,X,X,Start,Op,B,A]

init_ports:
	MOV #$FF, PTAD  ; safe load
	MOV #$00, PTADD ; PTA as INPUT (most lines, as 7,6 are restricted)
	; Notice that we wont need bit 7 and 6 of PTA, so no problem...
	
	; PTB will be for data
	; PTB has no deactivated bits, so no problem...
	MOV #$00, PTBD  ; safe load
	MOV #$00, PTDDD ; PTB as INPUT (all lines)

	; PTC will be for identifying the operation
	; PTC has no deactivated bits, so no problem...
	; We only really need 2 bits, though (4 possible cases/operations)
	MOV #$00, PTCD  ; safe load
	MOV #$FC, PTCDD ; PTC as INPUT (all lines)


;**************************************************************************************************
; Waiting for user input loop
;**************************************************************************************************

capture_wait:

	;********************************************
	; DEV MODE CODE
	;********************************************

	; For development purposes
	; so that it does something
	; without needing to operate the buttons/switches
	LDA #DEV_MODE
	CMP #1
	BNE no_force_start
	JMP start

no_force_start:
	; We will wait for button signals indicating what data is ready
	; Check if any of the buttons is pressed
	LDA PTAD
	AND #MASK_BUTTON
	BEQ capture_wait ; If Z=1, no button pressed, keep waiting
	JMP debounce     ; If not (Z=0), start capture what is in the switches

;**************************************************************************************************
; Capturing the user input
;**************************************************************************************************

;************************************************
; Debounce for buttons
;************************************************

debounce:
	LDHX #5000  ; The immediate you put here is the
				; is the size of the delay of the debounce
				; For development
delay:
	AIX #-1 ; Substract one
	CPHX #0 ; Check if we are already at zero
	BNE delay; Not Zero--> not ready, loop back
	BRA choose_capture ; We waited enough... go on with the capture

;************************************************
; Choosing what we are capturing
; based on what button was pressed
;************************************************

choose_capture:
	; The idea here is to detect which bit of PTAD is set
	; Remember PTA : [X,X,X,X,Start,Op,B,A]
	BRCLR 0, PTAD, capture_a
	BRCLR 1, PTAD, capture_b
	BRCLR 2, PTAD, capture_op
	BRCLR 3, PTAD, start
	JMP capture_wait

;************************************************
; Saving the user's input
;************************************************
; Depending on which is set, we just save the
; corresponding port in the associated variable
; And the we go back to waiting for user input

capture_a:
	MOV PTBD,OperandA
	JMP capture_wait

capture_b:
	MOV PTBD,OperandB
	JMP capture_wait

capture_op:
	MOV PTCD,Operator
	JMP capture_wait

;**************************************************************************************************
; Executing the operation
;**************************************************************************************************

start:

	;********************************************
	; Identify which operation the user wants
	; and branch accordingly
	;********************************************

	; Mask Operator to the LSBs
	LDA Operator
	AND #MASK_OP
	STA Operator ; Save the masked value so we dont have to keep doing that

	; Compare to know which route to take
	LDA Operator
	CMP #SUM_CODE
	BEQ _sum
	
	LDA Operator
	CMP #SUB_CODE
	BEQ _sub

	LDA Operator
	CMP #MUL_CODE
	BEQ _mul

	LDA Operator
	CMP #DIV_CODE
	BEQ _div

;**************************************************************************************************
; Operations
;**************************************************************************************************

;************************************************
; SUM
;************************************************
_sum:
	LDA #PLUS
	STA OpAscii ; For visualization

	MOV #0,Error      ; Assume there wont be a problem by default
	; A+B
	LDA OperandB
	ADD OperandA
check_sum: ; Checking if the sum has overflow
	; We need to check V (MSB of the CCR)
	; As I cannot directly check the V bit of the CCR
	; I need to load it in the accumulator

	TAX ; Temporally save Result (A)
	TPA ; Trasfer CCR to A
	; We must now check bit 7 (V)
	; This is equivalent to asking if A is negative (MSB ON)
	ADD #0 ; Update CCR
	BPL no_problem_sum ; If Positive, no overflow
	MOV #1,Error ; If negative, there was overflow. Turn On Error Variable
no_problem_sum:
	TXA ; Reload the copy of the result to the accumulator
	JMP show_result ; Go to the visualization subroutine

;************************************************
; SUBSTRACTION
;************************************************
_sub:
	; Self explanatory...
	LDA #MINUS
	STA OpAscii ; For visualization
	
	LDA OperandA
	SUB OperandB
	BRA check_sum ; Use the same overflow check as in the sum

;************************************************
; MULTIPLICATION
;************************************************

_mul:
	MOV #0,Error ; Assume there wont be a problem by default
	LDA #TIMES	 ; For visualization
	STA OpAscii


	; Notice MUL is only capable of doing unsigned multiplication
	; so first we get the sign-magnitude representation
	JSR get_A_B_sign_magnitude
	; At this point we have SignA-OperandAABS,SignB-OperandBABS set

	; Making the operation
	LDA OperandAABS
	LDX OperandBABS
	MUL
	STA ResultABS

	; Checking for an unrepresentable result
	; If (X:A[7]) != 0x0000:0b0, then result of the multiplication 
	; cannot be represented with 8bit signed integer
	; we need to check for this and feedback to the user
	
	; Checking X=$0000
	TXA ; If X is 0, we dont suspect there is a problem yet
	ADD #0 ; Force CCR update (TXA does not do it)
	BEQ check_muldiv_sign ; No problem detected
	MOV #1,Error ; Problem detected, multipication result unrepresentable in 8bits
	JMP check_muldiv_sign ; Jump to sign check for mul and div

;************************************************
; DIVISION
;************************************************
_div:
	MOV #0,Error ; Assume there wont be a problem by default
	LDA #DIVIDED ; For visualization
	STA OpAscii

	; Notice DIV is only capable of doing unsigned division
	; A / B
	; Divide, A <-- (H:A) / (X); H <- Remainder
	JSR get_A_B_sign_magnitude
	; At this point we have SignA-OperandAABS,SignB-OperandBABS set

	; Check B is not 0
	LDA OperandBABS
	BEQ division_by_zero

	; Doing de operation
	LDA OperandAABS
	LDHX #0 ; Clear H
	LDX OperandBABS
	DIV
	STA ResultABS

	JMP check_muldiv_sign ; Jump to sign check for mul and div
division_by_zero:
	AND #0 ; Clear A
	MOV #1,Error ; Have special led for zero division?
	JMP show_result

;************************************************
; SIGN HANDLING FOR MULTIPLICATION AND DIVISION
;************************************************

check_muldiv_sign:
	; Check the signed result is representable by 8bit 2's Complement
	; That is, the absolute value cannot be greater than 127
	; In other words, the last bit cannot be set
	; --> Cannot be negative when interpreted as a signed integer
	; NOTE: This logic misses the correct case of the multiplication being -128
	; we later check that the result is not the special case of -128

	; Checking if the result is positive when interpreted as a signed integer
	LDA ResultABS
	BPL get_muldiv_sign
	MOV #1,Error ; Not positive? Assume there is an error
	; The only case when it is correct is when the result is -128
	; Which will check later

get_muldiv_sign:
	; Get sign of the multiplication/division with exclusive or
	; Assumes SignA SignB Set
	; Asummes output in Result
	; Assumes ResultABS has the magnitude of the result
	; Save result sign at Sign Result
	; Save result AbsValue at ResultAbs
	; Saves signed result at Result

	; Assume Result's Sign is Positive
	; Thus ResultAbsoluteValue = Result
	; Change this otherwise
	LDA #PLUS
	STA SignResult
	LDA ResultABS
	STA Result

	; Obtaining A Sign
	; We need to mask as we used '+' and '-' chars
	LDA SignA
	AND #MASK_SIGN
	STA Aux

	; Obtaining B Sign
	LDA SignB ;
	AND #MASK_SIGN

	; Same Signs ---> Positive
	; Different Signs ---> Negative
	; We can check if the sign are the same or equal
	; Doing a xor of the masked bits of the ascii sign ('+','-')
	EOR Aux ; If Z=0-> Different Signs, If Z=1-> Same Signs
	STA SignResult ; Saving the Sign of the Result

	; If the sign is negative, change the supposition done before (which was being '+')
	; We can test if it is positive because in that case EOR its 0
	ADD #0         ; Force CCR Update
	BEQ handle_128 ; If sign is positive, we continue to handle the 128 case
	LDA #MINUS     ; Overwrite Sign with Negative
	STA SignResult
	               ; As the sign was negative, then Result has the wrong sign from the initial supposition
	LDA ResultABS
	NEGA           ; notice special case: NEG(-128)-->(-128)
	STA Result

handle_128:
	; Handle 128 Case
	; Check if Result is u128 and ResultSign is Negative (representable) 
	; Here we need to overwite de Error flagged set on before

	; Test if the number is 128 (positive or negative)
	LDA Result
	ADD #-128 ; u128 + (-)128=0
	BNE muldiv_ready ; If not 128 we are ready

	; Confirmed 128
	; Check Sign, Overwite Error Only if ResultSign = 1 (negative)
	LDA SignResult
	CMP #PLUS        ; Check if sign is positive, in that case we cannot represent it, which was our assumption
	BEQ muldiv_ready ; If Sign is Positive (A=0)--> +128--> Unrepresentable --> Keep the Error of Before, we are ready
	; Otherwise is 128 but negative, we can represent that so we overwrite the supposed error
	MOV #0,Error
muldiv_ready:
	LDA Result
	JMP show_result

;************************************************
; SIGN & MAGNITUDE Auxiliar Routines
;************************************************

get_A_B_sign_magnitude:
	; Subroutine
	; Modifies OperandAABS,SignA,OperandBABS,SignB,CCR
	; puts the Signs of A,B in SignA,SignB respectively
	; and makes OperandAABS,OperandBABS their absolute value
	LDA OperandA
	JSR get_sign_magnitude
	STA OperandAABS
	STX SignA
	LDA OperandB
	JSR get_sign_magnitude
	STA OperandBABS
	STX SignB
	RTS

get_sign_magnitude:
	; Assumes A is input in 2's complement
	; Saves sign at X (#PLUS or #MINUS symbols)
	; Saves uint at A
	ADD #0
	BPL is_positive
	BMI is_negative
is_positive:
	LDX #PLUS
	; A is already at correct representaion
	RTS
is_negative:
	NEGA
	LDX #MINUS
	RTS

;************************************************
; Binary to Decimal ASCII
;************************************************

bin_to_dec:
	; Assume input is at A
	; Modifies HX
	; Saves sign at Sign
	; Saves digit at [Hundreds,Tens,Ones]
	; The idea is to divide by a power of 10 to get the digit at that position
	; and using the remainder for the next digit

	; First get the sign-magnitude representation
	; As we need to divide by powers of 10 and that must be unsigned
	JSR get_sign_magnitude
	; At this point
	; X=SIGN , A=uint

	; Store sign
	STX Sign

	; Hundreds
	LDHX #0 ; Clear H for division
	LDX  #100 ; Divide by 100
	DIV ; A--> Division , H--> Remainder
	ADD #ASCII_OFFSET ; Offset to obtain the ascii character of the digit
	STA Hundreds

	; Tens
	STHX Aux ; Save previous remainder
	LDA Aux  ; Load remainder in A
	LDHX #0  ; Clean H for division
	LDX #10  ; Divide by 10
	DIV
	ADD #ASCII_OFFSET
	STA Tens

	; Ones
	; We already have the digit from the previous remainder
	STHX Aux ; Save previous Remainder
	LDA Aux  ; Load remainder in A
	ADD #ASCII_OFFSET
	STA Ones

	RTS

;************************************************
; Printing DECIMAL ASCII to a Memory Address
;************************************************

print:
	; Subroutine to print an ASCII representation of a number
	; At the memory position of AAscii + offset
	; AAscii+offset can represent AAscii,BAscii or RAscii
	; Assume:
	; - Aux = num to print
	; - Dummy = offset from AAscii

	; Convert number to decimal ascii representation
	LDA Aux
	JSR bin_to_dec
	; At this point we have set [Sign,Hundreds,Tens,Ones]
	
	; Define origin of print
	; Depending on the value of Dummy
	; AAscii+Dummy can represent AAscii,BAscii or RAscii
	LDHX #0 ; Is important to clear H here, something to do with the addresing mode...
	LDX Dummy

	; Sign
	; Is the first position so X is already at that point
	LDA Sign
	STA AAscii,X

	; Hundreds
	LDA Hundreds
	INCX ; Is just after the sign, so increase X by 1
	STA AAscii,X

	; Tens
	LDA Tens
	INCX ; Same logic, just after Hundreds so increase X by 1
	STA AAscii,X

	; Ones
	LDA Ones
	INCX ; Same logic, just after the Tens so increase X by 1
	STA AAscii,X

	RTS

;************************************************
; Showing the whole user message in memory
;************************************************

show_result:
	; Prints the message to the user in the memory
	; in a human-readable way
	; Message:
	; "OperandA [op] OperandB = Result"
	; Assume Accumulator = Signed Result

	STA Result

	; Printing Operand A
	LDA OperandA
	MOV #OFFSET_AAscii,Dummy
	STA Aux
	; Aux = Num to print, Dummy=Offset that points at AAscii
	JSR print

	; No need to print the operation
	; as Operation is already printed in each op's routine

	; Printing Operand B
	LDA OperandB
	MOV #OFFSET_BAscii,Dummy
	STA Aux
	; Aux = Num to print, Dummy=Offset that points at BAscii
	JSR print

	; Printing the equal sign (=)
	LDA #EQUAL
	STA EqualAscii

	; Printing the Result
	LDA Result
	MOV #OFFSET_RAscii,Dummy
	STA Aux
	; Aux = Num to print, Dummy=Offset that points at RAscii
	JSR print

	; Printing the error indicator

	; First assume there was no error
	; overwrite after
	LDA #NOERR
	STA ErrorAscii
	
	; Check if there was an error
	LDA Error
	CMP #1
	BNE ending  ; If no error, just continue to the ending
	LDA #ERR
	STA ErrorAscii ; Overwrite ErrorAscii in case of error
ending:
	nop
	JMP capture_wait ; wait for more input from the user


	ORG $FFFE
	DC.W _Startup ; Reset
