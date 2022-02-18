#include "derivative.h"

// Definiciones generales
#define			TRUE	1
#define			FALSE	0

#define			WAIT	__asm WAIT
#define			CLI		__asm CLI
#define			SEI		__asm SEI		 
#define			NOP		__asm NOP		 
															
// Tipos de datos
typedef unsigned char bool;

// Prototipos de Funciones
void SYSTEM_INIT(void);
void MCG_Init(void);