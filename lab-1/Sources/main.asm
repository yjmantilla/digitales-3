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
                ORG    Z_RAMStart ;$00B0
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
; ROM ES MEMORIA SOLO DE LECTURA 
		ORG   ROMStart ;  $1960
 _Startup:
 		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; Apagar WatchDOG
		LDA   #$20 ; LOAD ACCUM with $20, el porque lo explica en clase
		STA   SOPT1 ; STORE A en la direccion SOPT1
		;SOPT1 es $1802
		; Llevar el SP a la última posición de la RAM
		
		;Estas dos instrucciones previas es para inhabilitar el watchdog 
		; --> perro guardian, monitoreo?
		;un micro trabaja continuamente, ciclo infinito, for infinito
		;el watchdog reinicia en caso de bloqueos
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		
		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; MANEJO DEL STACK
		LDHX #RAMEnd+1
		TXS 		; Limpiar Registros
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		
		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; INicializacion limpia
		CLRA
		CLRX
		CLRH
		CLR Contador1
		CLR Contador2
		CLR Contador3
		CLR Contador4
		CLR Num_Estudiantes
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		;;; Carga de variables con valores reales;
		MOV #20, Num_Estudiantes ; trabajamos con 20 estudiantes
		;;; comenzamos con el tope y quitamos hasta llegar a 0
		
		;;;;; inicializacion de puertos
		;;; Poner en 0s los datos
		;;; colocar en entrada y salida los bits de los puertos de acuerdo al caso
		MOV #$00,PTAD
		MOV #$FF,PTADD
		MOV #$00,PTBD
		MOV #$FF, PTBDD
		MOV #$00,PTCD
		MOV #$FF, PTCDD
		MOV #$00,PTDD
		MOV #$FF, PTDDD
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Cuerpo del programa
Inicio: ;

again:      LDA    Tabla,X        ; Cargo el valor 
			; X tiene como el estudiante por donde va (creo), el indice
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
; Resto de bits es para las  notas
; Maximo puntaje es 50, pero el limite de la variable es 63 para esos 6 bits restantes
;;; Guardamos notas de estudiantes
Tabla:      DC.B   %10011101    ; Est. De Ing. de Telecomunic 10 , .score 29, TABLA +0 
            DC.B   %01100011    ; Est. De Ing. Eléctrica 01 , TABLA +1
            DC.B   %10110000    ; TABLA +2
            DC.B   %01110010    
            DC.B   %00101101    ; Est. De Ing. Electrónica 00 
            DC.B   %00101100    
            DC.B   %00100101    
            DC.B   %10101000    
            DC.B   %10100001    
            DC.B   %10000001    
            DC.B   %00100101    
            DC.B   %00110000    
            DC.B   %10001111    
            DC.B   %10011001    
            DC.B   %11011110  ; estudiante de Ing. Civil 11 
            DC.B   %11011101
            DC.B   %00101101    
            DC.B   %01110000    
            DC.B   %11001000    
            DC.B   %11000111    
          
                       
            

;*                 Interrupt Vectors                          *
;**************************************************************

            ORG	$FFFE

			

			
			DC.W  _Startup			; Reset
