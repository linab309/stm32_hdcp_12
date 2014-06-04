#ifndef _GLOBAL_H_50F385FE_A0C8_43A2_B3E1_2976C4C0FF04
#define _GLOBAL_H_50F385FE_A0C8_43A2_B3E1_2976C4C0FF04

#include "ChConfig.h"
#include <global/globalMacro.h>
#include <stdio.h>

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
PUTCHAR_PROTOTYPE;

#endif
