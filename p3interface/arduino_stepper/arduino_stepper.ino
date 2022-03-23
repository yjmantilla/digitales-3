/*------------------------------------CONTROL DE STEPPER MOTOR UNIPOLAR------------------------------------------------------------------
 * -------                  UNIVERSIDAD DE ANTIOQUIA                                                                  -------------------
 * -------                  FACULTAD DE INGENIERÍA ------- DEPARTAMENTO DE ING. ELECTRÓNICA                           -------------------
 * -------                  DESARROLLADO POR: ESAU BEDOYA, PARA: LABORATORIO ELECTRÓNICA ANÁLOGA II                    ------------------
 * -------------------------------------------------------------------------------------------------------------------------------------- */

//====Declaración de constantes y variables============================================================

#define BOBINAA 2   //Conecte a este pin (2) DIGITAL del Arduino el cable correspondiente a la bobina A
#define BOBINAB 3   //Conecte a este pin (3) DIGITAL del Arduino el cable correspondiente a la bobina B
#define BOBINAC 4   //Conecte a este pin (4) DIGITAL del Arduino el cable correspondiente a la bobina C
#define BOBINAD 5   //Conecte a este pin (5) DIGITAL del Arduino el cable correspondiente a la bobina D
#define VCC 8

bool ins=0, ind=0, ba=0, bb=0, bc=0, bd=0;
int pasos=0, sentido=0, frec=50, op=0, tem=0, t_paso=1;
float prec=2*5.625/64, ftem=0, pause=1000/200; 

//=====CONFIIGURACIÓN DE COMUNICACIONES Y DE PINES=====================================================
void setup() {
  

  Serial.begin(9600);
  pinMode(BOBINAA, OUTPUT);  //Configurar los pines de salida
  pinMode(BOBINAB, OUTPUT);
  pinMode(BOBINAC, OUTPUT);
  pinMode(BOBINAD, OUTPUT);
  pinMode(VCC, OUTPUT);
  digitalWrite(BOBINAA, LOW); //Configurar el estado inicial de las salidas
  digitalWrite(BOBINAB, LOW);
  digitalWrite(BOBINAC, LOW);
  digitalWrite(BOBINAD, LOW);
  digitalWrite(VCC, HIGH);
   
}


//====PROGRAMA PRINCIPAL================================================================================

void loop() {



  if(ins==0)
      inicializar(); //Dar instrucciones iniciales

   Serial.println("===============================================");
   Serial.println("=============MENÚ PRINCIPAL====================");
   Serial.println("");
   Serial.println("1. Sentido de giro");
   Serial.println("2. Configurar frecuencia.");
   Serial.println("3. Control por nro de pasos.");
   Serial.println("4. Control por grados.");
   Serial.println("5. Cambiar #grados/paso.");
   Serial.println("6. Seleccionar paso o medio paso.");
   Serial.println("Seleccione una opción.");
   while(Serial.available()==0){}
   op=Serial.parseInt();

   switch (op){

    case 1:
          Serial.println("Ingrese el sentido de giro: 0/horario 1/antihorario:");
          while(Serial.available()==0){}    //Hacer polling al serial
          tem=Serial.parseInt();            //Leer como entero el dato del serial
          if(tem==1){                       //Calsificar el sentido
            sentido=1;
            Serial.println("Sentido antihorario");
          }else if(tem==0){
            sentido=0;
            Serial.println("Sentido horario");
          }else
            Serial.println("opción no válida");
          break;

     case 2:
           Serial.println("Ingrese un valor entre 1 y 200");
           while(Serial.available()==0){}
           tem=Serial.parseInt();
           if(tem<=0 || tem>200)
            Serial.println("Fuera de rango.");
           else{
            frec=tem;
            pause=1000/(4*frec);        //Calcular la espera entre pasos en milisegundos
            Serial.println(frec);
           }
           break;

     case 3:
           Serial.println("Ingrese el nro de pasos a mover:");
           while(Serial.available()==0){}
           tem=Serial.parseInt();
           if(tem<=0)
            Serial.println("Valor incorrecto.");
           else{
              pasos=tem;
              Serial.println(pasos);
              if(t_paso==1){
                mediopaso(pasos);
              }else
                moverpasos(pasos);            //Control de movimiento
           }
           break;

     case 4:
           Serial.println("Ingrese los grados:");
           while(Serial.available()==0){}
           tem=Serial.parseInt();
           if(tem<=0)
            Serial.println("Valor incorrecto.");
           else{
              pasos=round(tem/prec);        //Conversión de grados a pasos para poder controlar
              Serial.println(tem);
              if(t_paso==1)
                mediopaso(2*pasos);
              else
                moverpasos(pasos);
           }
           break;

    case 5:
          Serial.println("Ingrese la nueva relación:");     //Modificar la relación, precisión del motor
          while(Serial.available()==0){}
          ftem=Serial.parseFloat();
          if(ftem<=0 || ftem>=1)
            Serial.println("Razón no válida");
          else{
            prec=ftem;
            Serial.println(prec);
          }
          break;
     case 6:
           Serial.println("Ingrese: 0/Paso completo 1/Medio paso:");
           while(Serial.available()==0){}
           tem=Serial.parseInt();
           if(tem==0){
            Serial.println("Paso completo.");
            t_paso=0;
           }else if(tem==1){
            Serial.println("Medio paso");
            t_paso=1;
           }
           else
            Serial.println("opcion no válida");
           break;
    default:
            Serial.println("Opción no válida.");
            break;
  }
  
}

//====INSTRUCCIONES INICIALES===============================================================================

void inicializar(){
  int con=0, mok=0;
  Serial.println("1. Conecte las los cables negro y blanco a la alimentacion (5-12V). Blanco V+.");
  Serial.println("2. Acople la tierra de la fuente con la de los pines DIGITAL del Arduino.");
  Serial.println("3. Conecte los cables de activacion de las bobinas: en los pines digitales 2,3,4,5");
  Serial.println("Presione una tecla y intro para continuar");
  while(Serial.available()==0){}
  (void)Serial.read();
  ins=1;
}

//=================Control de movimiento====================================================================

void moverpasos(int n){
  int i=0;
  ba=1; bb=0; bc=0; bd=0;
  
 
  for(i=1;i<=n;i++){      //control de la excitación de las bobinas, segun el número de pasos y según el sentido de giro.
    if(sentido==0){
      if(ba==1){
        digitalWrite(BOBINAD,LOW);
        digitalWrite(BOBINAA,HIGH);
        delay(pause);
        ba=0;
        bb=1;
      }else if(bb==1){
        digitalWrite(BOBINAA,LOW);
        digitalWrite(BOBINAB,HIGH);
        delay(pause);
        bb=0;
        bc=1;
      }else if(bc==1){
        digitalWrite(BOBINAB,LOW);
        digitalWrite(BOBINAC,HIGH);
        delay(pause);
        bc=0;
        bd=1;        
      }else if(bd==1){
        digitalWrite(BOBINAC,LOW);
        digitalWrite(BOBINAD,HIGH);
        delay(pause);
        bd=0;
        ba=1;
      }
    }else{
      if(ba==1){
        digitalWrite(BOBINAB,LOW);
        digitalWrite(BOBINAA,HIGH);
        delay(pause);
        ba=0;
        bd=1;
      }else if(bb==1){
        digitalWrite(BOBINAC,LOW);
        digitalWrite(BOBINAB,HIGH);
        delay(pause);
        bb=0;
        ba=1;
      }else if(bc==1){
        digitalWrite(BOBINAD,LOW);
        digitalWrite(BOBINAC,HIGH);
        delay(pause);
        bc=0;
        bb=1;        
      }else if(bd==1){
        digitalWrite(BOBINAA,LOW);
        digitalWrite(BOBINAD,HIGH);
        delay(pause);
        bd=0;
        bc=1;
      }
    }  
  }  
}

void mediopaso(int n){
  int i=0;
  ba=1; bb=0; bc=0; bd=0;
  for(i=1;i<=n;i++){
    if(sentido==0){
      if((ba==1 && bd==0) && bb==0){
         digitalWrite(BOBINAD,LOW);       
         digitalWrite(BOBINAA,HIGH);
         delay(pause);
         bb=1;
      }else if(ba==1 && bb==1){
        digitalWrite(BOBINAB,HIGH);
        delay(pause);
        ba=0;
      }else if(bb==1 && bc==0){
         digitalWrite(BOBINAA,LOW);
         delay(pause);
         bc=1;
      }else if(bb==1 && bc==1){
         digitalWrite(BOBINAC,HIGH);
         delay(pause);
         bb=0;
      }else if(bc==1 && bd==0){
        digitalWrite(BOBINAB,LOW);
        delay(pause);
        bd=1;
      }else if(bc==1 && bd==1){
        digitalWrite(BOBINAD,HIGH);
        delay(pause);
        bc=0;
      }else if(bd==1 && ba==0){
        digitalWrite(BOBINAC,LOW);
        delay(pause);
        ba=1;
      }else if(bd==1 && ba==1){
         digitalWrite(BOBINAA,HIGH);
         delay(pause);
         bd=0;
      }
    }else{
      if(ba==1 && bd==0 && bb==0){
         digitalWrite(BOBINAD,LOW);
         digitalWrite(BOBINAB,LOW);       
         digitalWrite(BOBINAA,HIGH);
         delay(pause);
         bd=1;
      }else if(ba==1 && bb==1){
        digitalWrite(BOBINAA,HIGH);
        delay(pause);
        bb=0;
      }else if(bb==1 && bc==0){
         digitalWrite(BOBINAC,LOW);
         delay(pause);
         ba=1;
      }else if(bb==1 && bc==1){
         digitalWrite(BOBINAB,HIGH);
         delay(pause);
         bc=0;
      }else if(bc==1 && bd==0){
        digitalWrite(BOBINAD,LOW);
        delay(pause);
        bb=1;
      }else if(bc==1 && bd==1){
        digitalWrite(BOBINAC,HIGH);
        delay(pause);
        bd=0;
      }else if(bd==1 && ba==0){
        digitalWrite(BOBINAA,LOW);
        delay(pause);
        bc=1;
      }else if(bd==1 && ba==1){
         digitalWrite(BOBINAD,HIGH);
         delay(pause);
         ba=0;
      }
    }    
  }
}
