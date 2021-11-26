; Include derivative-specific definitions
            INCLUDE 'derivative.inc' 
           	; este archivo se incluye en los headers
           	; incluye cosas del micro
; headers incluyen librerias y vainas
         
;
; XDEF es la direccion donde comienza
            XDEF _Startup ; esto es una pseudo instruccion???
            
            ABSENTRY _Startup

; mascara es comun para alterar algunos bits en particular
MASK            EQU %00111111

; Guardar vainas en memoria con labels a partir de una direccion inicial
; $ es hexadecimal
; PAGINA ZERO ES PREFERENCIAL RAPIDA
; estas variables se guardan en secuencia desde el origen de la ram
                ORG    Z_RAMStart
Contador1:        DS.B   1           ; Contador
Contador2:        DS.B   1 			; DS.B es un byte
Contador3:        DS.B   1
Contador4:        DS.B   1
Num_Estudiantes:  DS.B   1
;
; Sección para definir variables en memoria RAM, POR FUERA DE LA PAGINA ZERO
; NO TAN RAPIDA COMO LA PAGINA ZERO
            ORG    RAMStart ;$0100

; Sección de código del programa. 
		ORG   ROMStart ; ROM ES MEMORIA SOLO DE LECTURA $1960
 _Startup:
		; Apagar WatchDOG
		LDA   #$20
		STA   SOPT1
		; Llevar el SP a la última posición de la RAM
		LDHX #RAMEnd+1
		TXS 		; Limpiar Registros
		CLRA
		CLRX
		CLRH
		CLR Contador1
		CLR Contador2
		CLR Contador3
		CLR Contador4
		CLR Num_Estudiantes
		MOV #20, Num_Estudiantes
		MOV #$00,PTAD
		MOV #$FF,PTADD
		MOV #$00,PTBD
		MOV #$FF, PTBDD
		MOV #$00,PTCD
		MOV #$FF, PTCDD
		MOV #$00,PTDD
		MOV #$FF, PTDDD
; Cuerpo del programa
Inicio: ;

again:      LDA    Tabla,X        ; Cargo el valor  
            CMP    #64 
            BLO    Count1   ;branch if lower, for unsigned numb.
            CMP    #128
            BLO    Siga_2
            CMP    #192
            BLO    Siga_3
            AND    #MASK
            CMP    #30
            BLO    Siga_4
            BRA    Final
            
Count1:     CMP #45
            BNE Siga_1
            INC Contador1
Siga_1:     BRA Final

Siga_2:     AND #MASK 
            CMP #35
            BHS Count2
            BRA Final
Count2:     INC Contador2
            BRA Final
Siga_3:     AND #MASK
            CMP  #28
            BLS Final
            CMP #40
            BHI Final
            INC Contador3
            Bra Final      
Siga_4:     INC Contador4
Final:      INCX
            DBNZ Num_Estudiantes ,again
                   
            MOV Contador1, $00
            MOV Contador2, $02
            MOV Contador3, $04
            MOV Contador4, $06
Here:       BRA  Here

; los bits 7 y 6, identifican el programa del estudiante  
Tabla:      DC.B   %10011101    ; Est. De Ing. de Telecomunic.score 29
            DC.B   %01100011    ; Est. De Ing. Eléctrica
            DC.B   %10110000    
            DC.B   %01110010    
            DC.B   %00101101    ; Est. De Ing. Electrónica
            DC.B   %00101100    
            DC.B   %00100101    
            DC.B   %10101000    
            DC.B   %10100001    
            DC.B   %10000001    
            DC.B   %00100101    
            DC.B   %00110000    
            DC.B   %10001111    
            DC.B   %10011001    
            DC.B   %11011110  ; estudiante de Ing. Civil
            DC.B   %11011101
            DC.B   %00101101    
            DC.B   %01110000    
            DC.B   %11001000    
            DC.B   %11000111    
          
                       
            

;*                 Interrupt Vectors                          *
;**************************************************************

            ORG	$FFFE

			

			
			DC.W  _Startup			; Reset
