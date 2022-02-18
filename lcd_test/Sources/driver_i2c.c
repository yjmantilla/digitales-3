/******************************************************************************************
* Archivo: 			 driver_i2c.c (Driver Estándar)
* Descripción: 	 Funciones para leer / escribir en periféricos usando el bus I2C
* Autor: 				 Luis Germán García M.
* Fecha:         15/08/2007
* Actualización: 17/02/2008

* Para una mejor visualización, cambiar el Tab Size en las preferencias a 2
* Para un mejor entendimiento sobre el funcionamiento del bus I2C, vea el documento 
* adjunto .DOC
******************************************************************************************/

/******************************************************************************************
*
* Descripción
*
* NOTA: La información aquí presentada solo explica como trabaja este módulo.
*       Se recomienda leer el documento antes de entrar a utilizar las funciones aquí 
*       presentes. Allí se explica detalladamente como se maneja el bus I2C para escritura
*       y lectura de datos
*
* Las funciones aquí hacen uso del bus I2C, donde el MCU será siempre el dispositivo
* maestro y los demás períféricos siempre serán esclavos.
*
* Tales funciones de escritura y lectura pueden escribir o leer uno o varios datos. No se
* utilizan FIFOs, este manejador hace uso de arreglos de memoria. Máximo se pueden escribir
* o leer 255 datos. Se permite escribir o leer 0 datos. Lo anterior simplemente para
* comprobar la disponibilidad de un periférico en el bus.
*
* Si el periférico que se maneja no necesita, después de ser seleccionado, una dirección de
* memoria, entonces el parámetro wAddressMemory se puede ignorar.
* 
* Además, en los tiempos muertos de SCL y SDA, el programa principal quedará bloqueado. 
* En otras palabras, al llamar alguna de las funciones, estas amarran el programa desde 
* que se genera la condición de START hasta que se envía la condición de STOP.
*
* La frecuencia de trabajo de la línea SCL será de aproximadamente 50KHz. Tal frecuencia
* se genera haciendo uso de funciones de retraso usando instrucciones en ensamblador.
*
* Las funciones disponibles para el usuario son:
*
*  Write_Data_I2C : Escribe uno o varios datos en el dispositivo esclavo 
*  Read_Data_I2C  : Lee uno o varios datos desde el dispositivo esclavo
* 
*  Write_Data_EEPROM_I2C: Escribe uno o varios datos en una memoria EEPROM
*  Fill_Data_EEPROM_I2C:  Escribe el mismo valor en direcciones contiguas de la memoria EEPROM
*  Read_Data_EEPROM_I2C : Lee uno o varios datos en una memoria EEPROM
*
*  Las funciones para escritura en EEPROM permiten manejar entre 0 y 65535 datos.
*
*  Write_Data_RTC_I2C: Escribe uno o varios datos en el RTC
*  Read_Data_RTC_I2C : Lee uno o varios datos desde el RTC
*
********************************************************************************************/

#include "derivative.h"
#include "driver_i2c.h"

// Variables
static volatile char_i2c_tx_rx_str	_char_i2c_tx_rx;
volatile _STR_RTC  _var_rtc = {0};

/*******************************************************************************************
*                FUNCIONES PARA ESCRIBIR Y LEER DATOS USANDO PROTOCOLO I2C
*******************************************************************************************/

/*******************************************************************************************
* Escritura de datos: Write_Data_I2C
*
* A) Para escribir uno o varios datos se siguen estos pasos:
*  bAddressDevice = Dirección Dispositivo (7 Bits)
*  *bArrayFrames  = Puntero al arreglo de datos que se va a transmitir (Arreglo o Variable)
*  bNumFrames     = Número de datos a escribir (0 - 255)
* El programa realiza después de dar la orden de escritura:
*  1) Generar condición de START
*  2) Enviar la dirección del periférico y R/W = 0
*  3) Esperar por ACK
*  4) Enviar los n datos uno a uno, esperando ACK en cada uno de ellos
*  5) Enviar condición de STOP
*  Si la función retorna TRUE, los datos fueron enviados al Esclavo satisfactoriamente.
*
* B) Verificar la existencia de un periférico en el bus:
*  bAddressDevice = Dirección Dispositivo (7 Bits)
*  *bArrayFrames  = 0
*  bNumFrames     = 0
* El programa realiza después de dar la orden de escritura:
*  1) Generar condición de START
*  2) Enviar la dirección del periférico y R/W = 0
*  3) Esperar por ACK
*  4) Enviar condición de STOP
*  Si la función retorna TRUE, el esclavo está en el bus y responde.
*******************************************************************************************/
bool Write_Data_I2C(byte bAddressDevice, const byte *bArrayFrames, byte bNumFrames)
{
  byte i;
  
  // Preparar para escritura.
  bAddressDevice <<= 1;      // Bit 0, R/W = 0
  bAddressDevice &= 0xFE;    // Asegurar el CERO
    
  // Enviar condición de START
  _Start_I2C();
  
  // Enviar la dirección del periférico
  if (_Write_Frame_I2C(bAddressDevice) == FALSE)
  {
    _Stop_I2C();
    return FALSE;
  }
  
  // Enviar las tramas una a una
  for (i=0;i<bNumFrames;i++)
  {
    if (_Write_Frame_I2C(*bArrayFrames++) == FALSE)
    {
      _Stop_I2C();
      return FALSE;
    }
  }
  
  // Enviar condición de STOP
  _Stop_I2C();
  return TRUE;
}

/*******************************************************************************************
* Lectura de datos: Read_Data_I2C
*
* Para leer uno o varios datos se siguen estos pasos:
*  bAddressDevice = Dirección Dispositivo (7 Bits) 
*  *bArrayFrames  = Puntero a arreglo en memoria donde se escribirán los datos leídos (Arreglo o Variable)
*  bNumFrames     = Número de datos a leer (0 - 255)
* El programa realiza después de dar la orden de lectura:
*  1) Generar condición de START
*  2) Enviar la dirección del periférico y R/W = 1
*  3) Esperar por ACK
*  4) Leer los n datos uno a uno, enviando ACK en cada uno de ellos, menos en el último
*  5) Enviar condición de STOP
* La función retorna TRUE si los datos se leyeron satisfactoriamente
*******************************************************************************************/
bool Read_Data_I2C(byte bAddressDevice, byte *bArrayFrames, byte bNumFrames)
{
  byte i;
  byte ACK;
  
  bAddressDevice <<= 1;                 // Bit 0, R/W = 1
  bAddressDevice |= 0x01;               
    
  // Enviar condición de START
  _Start_I2C();
  
  // Enviar la dirección del periférico
  if (_Write_Frame_I2C(bAddressDevice) == FALSE)
  {
    _Stop_I2C();
    return FALSE;
  }
  
  // Recibir los datos uno a uno, observe que no se envía ACK en la última trama
  ACK = LOW_I2C;
  for (i=0;i<bNumFrames;i++)
  {
    if (i == (bNumFrames-1))
      ACK = HIGH_I2C;
    
    _Read_Frame_I2C(bArrayFrames++, ACK);
  }
  
  // Enviar condición de STOP
  _Stop_I2C();
  return TRUE;
}

/*******************************************************************************************
*           FUNCIONES ESPECIALES PARA ESCRIBIR Y LEER DATOS EN MEMORIAS EEPROM I2C
*******************************************************************************************/

/*******************************************************************************************
* Escritura de datos: Write_Data_EEPROM_I2C
*
* Para escribir uno o varios datos se siguen estos pasos:
*  bAddressDevice = Dirección Dispositivo (7 Bits)
*  *bArrayFrames  = Puntero al arreglo de datos que se va a transmitir (Arreglo o Variable)
*  wNumFrames     = Número de datos a escribir (0 - 65535)
*  wAddressMemory = Dirección de memoria de 16 bits desde donde se escriben los datos.
*  bMemorySize    = Se especifíca alguna de las siguientes etiquetas:
*     MEMORYSIZE_256BYTES, indica que se está empleando una memoria de 256 bytes de tamaño.
*     MEMORYSIZE_512BYTES, indica que se está empleando una memoria de 512 bytes de tamaño.
*     MEMORYSIZE_1024BYTES, indica que se está empleando una memoria de 1024 bytes de tamaño.
*     MEMORYSIZE_2048BYTES, indica que se está empleando una memoria de 2048 bytes de tamaño.
*     MEMORYSIZE_UP512KBYTES, indica que se está empleando una memoria de hasta 512 kbytes de tamaño.
*  bPageSize      = Se especifíca alguna de las siguientes etiquetas:
*     PAGESIZE_8BYTES, indica que la memoria tiene páginas de 8 bytes.
*     PAGESIZE_16BYTES, indica que la memoria tiene páginas de 16 bytes.
*     PAGESIZE_32BYTES, indica que la memoria tiene páginas de 32 bytes.
*     PAGESIZE_64BYTES, indica que la memoria tiene páginas de 64 bytes.
*     PAGESIZE_128BYTES, indica que la memoria tiene páginas de 128 bytes.
* El programa realiza después de dar la orden de escritura:
*  1) Generar condición de START
*  2) Enviar la dirección del periférico (y posiblemente bloque) y R/W = 0
*  3) Esperar por ACK
*  4) Enviar la dirección de memoria donde se escribirán los datos. Si se 
*     envió MEMORYSIZE_UP512KBYTES se mandan dos tramas de dirección.
*  5) Esperar por ACK
*  6) Enviar los n datos uno a uno, esperando ACK en cada uno de ellos.
*     Si los datos que se van a escribir se salen de la página, la función se
*     encarga de parar, esperar a que se graben los datos en la FLASH y continuar
*     en la siguiente página.
*  7) Enviar condición de STOP
*******************************************************************************************/
bool Write_Data_EEPROM_I2C(byte bAddressDevice, const byte *bArrayFrames, word wNumFrames, word wAddressMemory, byte bMemorySize, byte bPageSize)
{
  return _Write_Data_EEPROM_I2C(bAddressDevice, bArrayFrames, wNumFrames, wAddressMemory, bMemorySize, bPageSize, FALSE);
}

/*******************************************************************************************
* Escribir el mismo valor: Fill_Data_EEPROM_I2C
*
* Para escribir el mismo valor en varias posiciones de memoria contiguas:
*  bAddressDevice = Dirección Dispositivo (7 Bits)
*  bValToWrite    = Valor a escribir
*  wNumRegistersToFill = Número de datos a escribir (0 - 65535)
*  wAddressMemory = Dirección de memoria de 16 bits desde donde se rellena con el dato.
*  bMemorySize    = Se especifíca alguna de las siguientes etiquetas:
*     MEMORYSIZE_256BYTES, indica que se está empleando una memoria de 256 bytes de tamaño.
*     MEMORYSIZE_512BYTES, indica que se está empleando una memoria de 512 bytes de tamaño.
*     MEMORYSIZE_1024BYTES, indica que se está empleando una memoria de 1024 bytes de tamaño.
*     MEMORYSIZE_2048BYTES, indica que se está empleando una memoria de 2048 bytes de tamaño.
*     MEMORYSIZE_UP512KBYTES, indica que se está empleando una memoria de hasta 512 kbytes de tamaño.
*  bPageSize      = Se especifíca alguna de las siguientes etiquetas:
*     PAGESIZE_8BYTES, indica que la memoria tiene páginas de 8 bytes.
*     PAGESIZE_16BYTES, indica que la memoria tiene páginas de 16 bytes.
*     PAGESIZE_32BYTES, indica que la memoria tiene páginas de 32 bytes.
*     PAGESIZE_64BYTES, indica que la memoria tiene páginas de 64 bytes.
*     PAGESIZE_128BYTES, indica que la memoria tiene páginas de 128 bytes.
* El programa realiza después de dar la orden de escritura:
*  1) Generar condición de START
*  2) Enviar la dirección del periférico (y posiblemente bloque) y R/W = 0
*  3) Esperar por ACK
*  4) Enviar la dirección de memoria donde se escribirán los datos. Si se 
*     envió MEMORYSIZE_UP512KBYTES se mandan dos tramas de dirección.
*  5) Esperar por ACK
*  6) Enviar los n datos uno a uno, esperando ACK en cada uno de ellos.
*     Si los datos que se van a escribir se salen de la página, la función se
*     encarga de parar, esperar a que se graben los datos en la FLASH y continuar
*     en la siguiente página.
*  7) Enviar condición de STOP
*******************************************************************************************/
bool Fill_Data_EEPROM_I2C(byte bAddressDevice, byte bValToWrite, word wNumRegistersToFill, word wAddressMemory, byte bMemorySize, byte bPageSize)
{
  return _Write_Data_EEPROM_I2C(bAddressDevice, &bValToWrite, wNumRegistersToFill, wAddressMemory, bMemorySize, bPageSize, TRUE);
}

/*******************************************************************************************
* Función para escribir datos en la memoria EEPROM. Función esclava.
*******************************************************************************************/
static bool _Write_Data_EEPROM_I2C(byte bAddressDevice, const byte *bArrayFrames, word wNumFrames, word wAddressMemory, byte bMemorySize, byte bPageSize, byte bFillData)
{
  word i;
  byte bModAddressDevice;
  bool WriteMorePages = FALSE;

  // Escribir las páginas necesarias
  do
  {

    bModAddressDevice = bAddressDevice;
    
    // Para las memorias de 2 a 8 bloques, éste se introduce dentro de la dirección del dispositivo
    if (bMemorySize == MEMORYSIZE_512BYTES)
    {
      bModAddressDevice &= 0xFE;
      bModAddressDevice |= ((wAddressMemory >> 8) & 0x01);
    }
    else if (bMemorySize == MEMORYSIZE_1024BYTES)
    {
      bModAddressDevice &= 0xFC;
      bModAddressDevice |= ((wAddressMemory >> 8) & 0x03);
    }
    else if (bMemorySize == MEMORYSIZE_2048BYTES)
    {
      bModAddressDevice &= 0xF8;
      bModAddressDevice |= ((wAddressMemory >> 8) & 0x07);
    }

    /* Verificar que la memoria EEPROM esté disponible, intentar por espacio  */
    /* de 11ms, posiblemente hubo escritura previamente en la memoria y en    */
    /* el momento se encuentra ocupada.                                       */
    /* El siguiente bucle gasta 1122 ciclos por iteración.                   */
    /* NUM_INT_WAIT, número de veces que se ejecuta el for para generar 11ms  */
    /* NUM_INT_WAIT = 11ms*Fbus/1122                                          */
    for (i=0;i<NUM_INT_WAIT;i++)
      if (Write_Data_I2C(bModAddressDevice, 0, 0) == TRUE) break;

    // Preparar para escritura, no se hace antes por que Write_Data_I2C Espera 7-bits.
    bModAddressDevice <<= 1;                       // Bit 0, R/W = 0
    bModAddressDevice &= 0xFE;

    // Enviar condición de START
    _Start_I2C();

    // Enviar la dirección del periférico
    if (_Write_Frame_I2C(bModAddressDevice) == FALSE)
    {
      _Stop_I2C();
      return FALSE;
    }

    // Para memorias de más de 8 bloques, se envían dos tramas de dirección
    if (bMemorySize == MEMORYSIZE_UP512KBYTES)
    {
      // Se envía el bloque
      if (_Write_Frame_I2C((wAddressMemory >> 8) & 0xFF) == FALSE)
      {
        _Stop_I2C();
        return FALSE;
      }
    }
    // Se envía dirección dentro del bloque n de la memoria
    if (_Write_Frame_I2C(wAddressMemory & 0xFF) == FALSE)
    {
      _Stop_I2C();
      return FALSE;
    }

    // Enviar las tramas una a una (teniendo en cuenta las páginas)
    WriteMorePages = FALSE;
    for (i=0;i<wNumFrames;i++)
    {
      // Verificar si se va a saltar de página
      if ((word)(i+wAddressMemory) % bPageSize == 0 && i)
      {
        // Cálculos para saltar a la siguiente página
        wNumFrames -= i;
        wAddressMemory += i;
        WriteMorePages = TRUE;
        
        break;     // Romper este for, salto a la siguiente página
      }

      // Escribir trama
      if (bFillData == FALSE)
      {
        // Escribir siguiente dato
        if (_Write_Frame_I2C(*bArrayFrames++) == FALSE)
        {
          _Stop_I2C();
          return FALSE;
        }
      }
      else
      {
        // Colocar el mismo valor en todas las posiciones
        if (_Write_Frame_I2C(*bArrayFrames) == FALSE)
        {
          _Stop_I2C();
          return FALSE;
        }
      }

    }

    // Enviar condición de STOP
    _Stop_I2C();
    
  } while (WriteMorePages == TRUE); // Continuar si faltan páginas por grabar

  return TRUE;
}

/*******************************************************************************************
* Lectura de datos: Read_Data_EEPROM_I2C
*
* Para leer uno o varios datos se siguen estos pasos:
*  bAddressDevice = Dirección Dispositivo (7 Bits)
*  *bArrayFrames  = Puntero al arreglo de datos donde se van a escribir los datos leídos
*  bNumFrames     = Número de datos a leer (0 - 255)
*  wAddressMemory = Dirección de memoria de 16 bits donde se empiezan a leer los datos.
*  bMemorySize    = Se especifíca alguna de las siguientes etiquetas:
*     MEMORYSIZE_256BYTES, indica que se está empleando una memoria de 256 bytes de tamaño.
*     MEMORYSIZE_512BYTES, indica que se está empleando una memoria de 512 bytes de tamaño.
*     MEMORYSIZE_1024BYTES, indica que se está empleando una memoria de 1024 bytes de tamaño.
*     MEMORYSIZE_2048BYTES, indica que se está empleando una memoria de 2048 bytes de tamaño.
*     MEMORYSIZE_UP512KBYTES, indica que se está empleando una memoria de hasta 512 kbytes de tamaño.
* El programa realiza después de dar la orden de escritura:
*  1) Generar condición de START
*  2) Enviar la dirección del periférico (y posiblemente bloque) y R/W = 0
*  3) Esperar por ACK
*  4) Enviar la dirección de memoria desde donde se van a leer los datos. Si se 
*     envió MEMORYSIZE_UP512KBYTES se mandan dos tramas de dirección.
*  5) Esperar por ACK
*  6) Se envía condición de Re-Start.
*  7) Enviar la dirección del periférico y R/W = 1
*  8) Leer n datos uno a uno, enviando ACK en cada uno de ellos, excepto en el último.
*  9) Enviar condición de STOP
*
********************************************************************************************/
bool Read_Data_EEPROM_I2C(byte bAddressDevice, byte *bArrayFrames, byte bNumFrames, word wAddressMemory, byte bMemorySize)
{
  byte i;
  byte ACK;
  
  // Para las memorias de 2 a 8 bloques, éste se introduce dentro de la dirección del dispositivo
  if (bMemorySize == MEMORYSIZE_512BYTES)
  {
    bAddressDevice &= 0xFE;
    bAddressDevice |= ((wAddressMemory >> 8) & 0x01);
  }
  else if (bMemorySize == MEMORYSIZE_1024BYTES)
  {
    bAddressDevice &= 0xFC;
    bAddressDevice |= ((wAddressMemory >> 8) & 0x03);
  }
  else if (bMemorySize == MEMORYSIZE_2048BYTES)
  {
    bAddressDevice &= 0xF8;
    bAddressDevice |= ((wAddressMemory >> 8) & 0x07);
  }
    
  /* Verificar que la memoria EEPROM esté disponible, intentar por espacio  */
  /* de 11ms, posiblemente hubo escritura previamente en la memoria y en    */
  /* el momento se encuentra ocupada.                                       */
  /* El siguiente bucle gasta 1122 ciclos por iteracción.                   */
  /* NUM_INT_WAIT, número de veces que se ejecuta el for para generar 11ms  */
  /* NUM_INT_WAIT = 11ms*Fbus/1122                                          */
  for (i=0;i<NUM_INT_WAIT;i++)
    if (Write_Data_I2C(bAddressDevice, 0, 0) == TRUE) break;

  // Preparar para escritura.
  bAddressDevice <<= 1;                 // Bit 0, R/W = 0
  bAddressDevice &= 0xFE;
    
  // Enviar condición de START
  _Start_I2C();
  
  // Enviar la dirección del periférico
  if (_Write_Frame_I2C(bAddressDevice) == FALSE)
  {
    _Stop_I2C();
    return FALSE;
  }
  
  // Para memorias de más de 16 bloques, se envían dos tramas de dirección
  if (bMemorySize == MEMORYSIZE_UP512KBYTES)
  {
    // Se envía el bloque
    if (_Write_Frame_I2C((wAddressMemory >> 8) & 0xFF) == FALSE)
    {
      _Stop_I2C();
      return FALSE;
    }
  }
  // Se envía dirección dentro del bloque
  if (_Write_Frame_I2C(wAddressMemory & 0xFF) == FALSE)
  {
    _Stop_I2C();
    return FALSE;
  }

  // Enviar condición de Re-Start
  _Re_Start_I2C();

  // Enviar la dirección del periférico con R/W = 1
  bAddressDevice |= 0x01;               
  if (_Write_Frame_I2C(bAddressDevice) == FALSE)
  {
    _Stop_I2C();
    return FALSE;
  }
  
  // Recibir los datos uno a uno, observe que no se envía ACK en la última trama
  ACK = LOW_I2C;
  for (i=0;i<bNumFrames;i++)
  {
    if (i == (bNumFrames-1))
      ACK = HIGH_I2C;
    
    _Read_Frame_I2C(bArrayFrames++, ACK);
  }
  
  // Enviar condición de STOP
  _Stop_I2C();
  return TRUE;
}

/*******************************************************************************************
*                FUNCIONES ESPECIALES PARA ESCRIBIR Y LEER DATOS EN RTC I2C
*******************************************************************************************/

/*******************************************************************************************
* Escritura de datos: Write_Data_RTC_I2C
*
* Para escribir uno o varios datos se siguen estos pasos:
*  bAddressDevice = Dirección Dispositivo (7 Bits)
*  *bArrayFrames  = Puntero al arreglo de datos que se va a transmitir (Arreglo o Variable)
*  bNumFrames     = Número de datos a escribir (0 - 255)
*  bAddressMemory = Dirección de memoria de 8 bits desde donde se escriben los datos.
* El programa realiza después de dar la orden de escritura:
*  1) Generar condición de START
*  2) Enviar la dirección del periférico y R/W = 0
*  3) Esperar por ACK
*  4) Enviar la dirección de memoria donde se escribirán los datos.
*  5) Esperar por ACK
*  6) Enviar los n datos uno a uno, esperando ACK en cada uno de ellos.
*  7) Enviar condición de STOP
*******************************************************************************************/
bool Write_Data_RTC_I2C(byte bAddressDevice, const byte *bArrayFrames, byte bNumFrames, byte bAddressMemory)
{
  byte i;

  /* Verificar que el RTC esté disponible, intentar por espacio de 11ms.    */
  /* El siguiente bucle gasta 1122 ciclos por iteracción.                   */
  /* NUM_INT_WAIT, número de veces que se ejecuta el for para generar 11ms  */
  /* NUM_INT_WAIT = 11ms*Fbus/1122                                          */
  for (i=0;i<NUM_INT_WAIT;i++)
    if (Write_Data_I2C(bAddressDevice, 0, 0) == TRUE) break;

  // Preparar para escritura, no se hace antes por que Write_Data_I2C espera 7-bits.
  bAddressDevice <<= 1;                       // Bit 0, R/W = 0
  bAddressDevice &= 0xFE;

  // Enviar condición de START
  _Start_I2C();

  // Enviar la dirección del periférico
  if (_Write_Frame_I2C(bAddressDevice) == FALSE)
  {
    _Stop_I2C();
    return FALSE;
  }

  // Se envía dirección
  if (_Write_Frame_I2C(bAddressMemory & 0xFF) == FALSE)
  {
    _Stop_I2C();
    return FALSE;
  }

  // Enviar las tramas una a una
  for (i=0;i<bNumFrames;i++)
  {
    // Escribir siguiente dato
    if (_Write_Frame_I2C(*bArrayFrames++) == FALSE)
    {
      _Stop_I2C();
      return FALSE;
    }
  }

  // Enviar condición de STOP
  _Stop_I2C();
    
  return TRUE;
}

/*******************************************************************************************
* Lectura de datos: Read_Data_RTC_I2C
*
* Para leer uno o varios datos se siguen estos pasos:
*  bAddressDevice = Dirección Dispositivo (7 Bits)
*  *bArrayFrames  = Puntero al arreglo de datos donde se van a escribir los datos leídos
*  bNumFrames     = Número de datos a leer (0 - 255)
*  bAddressMemory = Dirección de memoria de 8 bits donde se empiezan a leer los datos.
* El programa realiza después de dar la orden de escritura:
*  1) Generar condición de START
*  2) Enviar la dirección del periférico y R/W = 0
*  3) Esperar por ACK
*  4) Enviar la dirección de memoria desde donde se van a leer los datos.
*  5) Esperar por ACK
*  6) Se envía condición de Re-Start.
*  7) Enviar la dirección del periférico y R/W = 1
*  8) Leer n datos uno a uno, enviando ACK en cada uno de ellos, excepto en el último.
*  9) Enviar condición de STOP
*
********************************************************************************************/
bool Read_Data_RTC_I2C(byte bAddressDevice, byte *bArrayFrames, byte bNumFrames, byte bAddressMemory)
{
  byte i;
  byte ACK;
  
  /* Verificar que el RTC esté disponible, intentar por espacio de 11ms.    */
  /* El siguiente bucle gasta 1122 ciclos por iteracción.                   */
  /* NUM_INT_WAIT, número de veces que se ejecuta el for para generar 11ms  */
  /* NUM_INT_WAIT = 11ms*Fbus/1122                                          */
  for (i=0;i<NUM_INT_WAIT;i++)
    if (Write_Data_I2C(bAddressDevice, 0, 0) == TRUE) break;

  // Preparar para escritura.
  bAddressDevice <<= 1;                 // Bit 0, R/W = 0
  bAddressDevice &= 0xFE;
    
  // Enviar condición de START
  _Start_I2C();
  
  // Enviar la dirección del periférico
  if (_Write_Frame_I2C(bAddressDevice) == FALSE)
  {
    _Stop_I2C();
    return FALSE;
  }
  
  // Se envía dirección
  if (_Write_Frame_I2C(bAddressMemory & 0xFF) == FALSE)
  {
    _Stop_I2C();
    return FALSE;
  }

  // Enviar condición de Re-Start
  _Re_Start_I2C();

  // Enviar la dirección del periférico con R/W = 1
  bAddressDevice |= 0x01;               
  if (_Write_Frame_I2C(bAddressDevice) == FALSE)
  {
    _Stop_I2C();
    return FALSE;
  }
  
  // Recibir los datos uno a uno, observe que no se envía ACK en la última trama
  ACK = LOW_I2C;
  for (i=0;i<bNumFrames;i++)
  {
    if (i == (bNumFrames-1))
      ACK = HIGH_I2C;
    
    _Read_Frame_I2C(bArrayFrames++, ACK);
  }
  
  // Enviar condición de STOP
  _Stop_I2C();
  return TRUE;
}


/*******************************************************************************************
*              SUBFUNCIONES PARA GENERACIÓN Y LECTURA DE TRAMAS I2C
*******************************************************************************************/

// Esta función genera la condición de START y deja la línea SCL en 0
static void _Start_I2C(void)
{
  // Tomar control del Bus I2C
  SDA = HIGH_I2C;
  OUT_SDA();
  Delay_I2C();
  SCL = HIGH_I2C;
  OUT_SCL();
  Delay_I2C();
  
  // Generar condición de Start
  SDA = LOW_I2C;
  Delay_I2C();
  SCL = LOW_I2C;
}

// Esta función se encarga de generar una nueva condición de Start
static void _Re_Start_I2C(void)
{
  // Generar condición de Start
  _Start_I2C();
}

// Esta función es usada para escribir 8 bits en cada trama además de esperar el respectivo
// Acknowledgment. La línea SCL se deja en 0 al finalizar
static bool _Write_Frame_I2C(const byte bData)
{
  byte i;
  
  char_i2c_tx_rx = bData;
  
  // ***********************************
  // Envía bit a bit usando la línea SDA
  // ***********************************
  for (i=0;i<=7;i++)
  {
    // Poner en bit en SDA
    SDA = char_i2c_tx_rx_msb;
    char_i2c_tx_rx <<= 1;
    // Esperar para cambiar SCL de 0 a 1
    Delay_I2C();      
    // Poner SCL a 1
    SCL = HIGH_I2C;
    // Esperar para cambiar SCL de 1 a 0
    Delay_I2C();
    // Poner SCL a 0
    SCL = LOW_I2C;
  }
  
  // ***********
  // Recibir ACK
  // ***********
  
  // Evitar corto en el pin de SDA
  SDA = LOW_I2C;
  // Limpiar buffer
  char_i2c_tx_rx = 0;
  // SDA como entrada
  IN_SDA();
  // Esperar para cambiar SCL de 0 a 1
  Delay_I2C();
  // Poner SCL a 1
  SCL = HIGH_I2C;
  // Esperar para cambiar SCL de 1 a 0
  Delay_I2C();
  // Tomar el valor de SDA
  char_i2c_tx_rx_lsb = SDA;
  // Poner SCL a 0
  SCL = LOW_I2C;
  // SDA como salida
  OUT_SDA();
  // Verificar el valor de ACK
  if (char_i2c_tx_rx_lsb == LOW_I2C)
    return TRUE;
  else
    return FALSE;
}

// Esta función es usada para recibir 8 bits de cada trama además de generar el ACK
// La línea SCL se deja en 0
static void _Read_Frame_I2C(byte *bData, byte bACK)
{
  byte i;
  
  // Limpiar buffer
  char_i2c_tx_rx = 0;
  // Evitar corto en el pin de SDA
  SDA = LOW_I2C;
  
  // SDA como entrada
  IN_SDA();
  
  // ************************************
  // Recibe bit a bit usando la línea SDA
  // ************************************
  for (i=0;i<=7;i++)
  {
    // Esperar para cambiar SCL de 0 a 1
    Delay_I2C();      
    // Poner SCL a 1
    SCL = HIGH_I2C;
    // Esperar para tomar el bit por SDA y cambiar SCL de 1 a 0
    Delay_I2C();
    char_i2c_tx_rx <<= 1;
    char_i2c_tx_rx_lsb = SDA;
    // Poner SCL a 0
    SCL = LOW_I2C;
  }
  
  // Dato leido
  *bData = char_i2c_tx_rx;
  
  // ************
  // Escribir ACK
  // ************
  SDA = bACK;
  OUT_SDA();
  // Esperar para cambiar SCL de 0 a 1
  Delay_I2C();
  // Poner SCL a 1
  SCL = HIGH_I2C;
  // Esperar para cambiar SCL de 1 a 0
  Delay_I2C();
  // Poner SCL a 0
  SCL = LOW_I2C;
}

// Esta función genera la condición de STOP y deja la línea SCL como entrada
static void _Stop_I2C(void)
{
  // Llevar SDA a 0
  SDA = LOW_I2C;
  // Esperar para cambiar SCL de 0 a 1
  Delay_I2C();
  // Llevar SCL a 1
  SCL = HIGH_I2C;
  // Esperar para cambiar SDA de 0 a 1
  Delay_I2C();
  // Llevar SDA a 1
  SDA = HIGH_I2C;
  // Liberar el Bus
  IN_SCL();
  IN_SDA();
}

// Se generan retrasos usando instrucciones en ensamblador. Se emplea la instrucción JSR 
// para invocar esta función, la cual toma 6 ciclos de máquina. Además, la instrucción RTS
// toma 5 ciclos. Dependiendo del retraso a generar, se coloca un número de instrucciones
// NOP. La ecuacíón es: (6+5+X)/Frecuencia_Bus = Tiempo Retraso. X es el número de veces que
// debe aparecer la instrucción NOP: X = (Tiempo_Retraso*Frecuencia_Bus)-11
// Con X = 109, el retraso de esta función es de 10us para una frecuencia de bus de 12MHz
static void _Delay_I2C (void)
{
  #define INST_NOP()   NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;

  INST_NOP();  // Ejecución de varias instrucciones NOP
  INST_NOP();  // Ejecución de varias instrucciones NOP
  INST_NOP();  // Ejecución de varias instrucciones NOP
  INST_NOP();  // Ejecución de varias instrucciones NOP
  INST_NOP();  // Ejecución de varias instrucciones NOP
  INST_NOP();  // Ejecución de varias instrucciones NOP
  INST_NOP();  // Ejecución de varias instrucciones NOP
  INST_NOP();  // Ejecución de varias instrucciones NOP
  INST_NOP();  // Ejecución de varias instrucciones NOP
  INST_NOP();  // Ejecución de varias instrucciones NOP
  NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP; NOP;
}
