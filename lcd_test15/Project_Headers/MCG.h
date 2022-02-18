#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */

#define BUS12M
#define MAX_COUNT 250

void MCG_Init(void);