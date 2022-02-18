/**************************************************************************
* Archivo: 			 driver_i2c.h
* Descripción: 	 Definiciones para el manejo del bus I2C
* Autor: 				 Luis Germán García M.
* Fecha:         15/08/2007
* Actualización: 17/02/2008

* Para una mejor visualización, cambiar el Tab Size en las preferencias a 2
***************************************************************************/

#include "derivative.h"
#include "general.h"

// Definiciones generales
#define Delay_I2C()     __asm JSR _Delay_I2C;

#define LOW_I2C		0
#define HIGH_I2C	1

// Pines del MCU para SDA y SCL
#define DDR_SDA		PTADD_PTADD4
#define DDR_SCL		PTADD_PTADD5
#define SDA				PTAD_PTAD4
#define SCL				PTAD_PTAD5

#define IN_SDA()		DDR_SDA = 0
#define IN_SCL()		DDR_SCL = 0
#define OUT_SDA()		DDR_SDA = 1
#define OUT_SCL()		DDR_SCL = 1

// Tiempo de espera, mientras la memoria EEPROM realiza escritura de datos
// en la ROM
#define TIME_WAIT_MS    11
#define FBUS_HC08       12000000
#define NUM_INT_WAIT    (TIME_WAIT_MS*FBUS_HC08)/1122000  // Operación hecha por C

// Prototipos de funciones
bool Write_Data_I2C(byte, const byte *, byte);
bool Read_Data_I2C(byte, byte *, byte);
bool Write_Data_EEPROM_I2C(byte, const byte *, word, word, byte, byte);
bool Fill_Data_EEPROM_I2C(byte, byte, word, word, byte, byte);
bool Read_Data_EEPROM_I2C(byte, byte *, byte, word, byte);
bool Write_Data_RTC_I2C(byte, const byte *, byte, byte);
bool Read_Data_RTC_I2C(byte, byte *, byte, byte);
static bool _Write_Data_EEPROM_I2C(byte, const byte *, word, word, byte, byte, byte);
static void _Start_I2C(void);
static void _Re_Start_I2C(void);
static bool _Write_Frame_I2C(const byte);
static void _Read_Frame_I2C(byte *, byte);
static void _Stop_I2C(void);
static void _Delay_I2C(void);

// Macros para el parámetro bMemorySize
#define MEMORYSIZE_256BYTES    0   // Memoria sin bloques (24LC02)
#define MEMORYSIZE_512BYTES    1   // Memoria con hasta 2 bloques (24LC04)
#define MEMORYSIZE_1024BYTES   2   // Memoria con hasta 4 bloques (24LC08)
#define MEMORYSIZE_2048BYTES   3   // Memoria con hasta 8 bloques (24LC16)
#define MEMORYSIZE_UP512KBYTES 4   // Memoria con hasta 256 bloques

// Macros para el parámetro bPageSize
#define PAGESIZE_8BYTES        8
#define PAGESIZE_16BYTES      16 
#define PAGESIZE_32BYTES      32
#define PAGESIZE_64BYTES      64
#define PAGESIZE_128BYTES    128

// Estructura y macros para RTC DS1307
typedef struct
{
  // Segundos
  unsigned char UNI_SEG :4;
  unsigned char DEC_SEG :3;
  unsigned char CH      :1;
  // Minutos
  unsigned char UNI_MIN :4;
  unsigned char DEC_MIN :3;
  unsigned char         :1;
  // Horas
  union 
  {
    struct 
    {
      // Horas 24
      unsigned char UNI_HOR :4;
      unsigned char DEC_HOR :2;
      unsigned char _12_24  :1;
      unsigned char         :1;
    }_24;
    struct 
    {
      // Horas 12
      unsigned char UNI_HOR :4;
      unsigned char DEC_HOR :1;
      unsigned char AM_PM   :1;
      unsigned char         :2;
    }_12;
  }_HORA;
  // Día de la semana
  unsigned char DIASEM  :3;
  unsigned char         :5;
  // Día del mes
  unsigned char UNI_DIA :4;
  unsigned char DEC_DIA :2;
  unsigned char         :2;
  // Mes
  unsigned char UNI_MES :4;
  unsigned char DEC_MES :1;
  unsigned char         :3;
  // Año
  unsigned char UNI_YEA :4;
  unsigned char DEC_YEA :4;
}_STR_RTC;

extern volatile _STR_RTC  _var_rtc;
#define RTC_CH         _var_rtc.CH
#define RTC_UNI_SEG    _var_rtc.UNI_SEG
#define RTC_DEC_SEG    _var_rtc.DEC_SEG
#define RTC_UNI_MIN    _var_rtc.UNI_MIN
#define RTC_DEC_MIN    _var_rtc.DEC_MIN
#define RTC_12_24      _var_rtc._HORA._24._12_24
#define RTC_UNI_HOUR24 _var_rtc._HORA._24.UNI_HOR
#define RTC_DEC_HOUR24 _var_rtc._HORA._24.DEC_HOR
#define RTC_UNI_HOUR12 _var_rtc._HORA._12.UNI_HOR
#define RTC_DEC_HOUR12 _var_rtc._HORA._12.DEC_HOR
#define RTC_AM_PM      _var_rtc._HORA._12.AM_PM
#define RTC_DIASEM     _var_rtc.DIASEM
#define RTC_UNI_DIA    _var_rtc.UNI_DIA
#define RTC_DEC_DIA    _var_rtc.DEC_DIA
#define RTC_UNI_MES    _var_rtc.UNI_MES
#define RTC_DEC_MES    _var_rtc.DEC_MES
#define RTC_UNI_YEA    _var_rtc.UNI_YEA
#define RTC_DEC_YEA    _var_rtc.DEC_YEA

#define RTC_FORMAT_12H    1
#define RTC_FORMAT_24H    0

#define RTC_12H_AM        0
#define RTC_12H_PM        1
  
// Variables para el manejo de I2C
typedef union
{
	byte Byte;
	struct
	{
		byte LSB:			1;
		byte		:			6;
		byte MSB:			1;
	}Bits;
}char_i2c_tx_rx_str;

// static volatile char_i2c_tx_rx_str	_char_i2c_tx_rx;
#define char_i2c_tx_rx			_char_i2c_tx_rx.Byte
#define char_i2c_tx_rx_msb	_char_i2c_tx_rx.Bits.MSB
#define char_i2c_tx_rx_lsb	_char_i2c_tx_rx.Bits.LSB
