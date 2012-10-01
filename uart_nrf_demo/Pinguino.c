#include <p32xxxx.h>
#include <plib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myspi.h"
#include "Pinguino.h"


void
delay_1ms (void)
{
#define DELAY 156
  T1CON = 0x8030;
  PR1 = 0xffff;
  TMR1 = 0;
  while (TMR1 < DELAY);


  return;
}

void
delay_7us (void)
{
#define DELAYU 1
  T1CON = 0x8030;
  PR1 = 0xffff;
  TMR1 = 0;
  while (TMR1 < DELAYU);


  return;
}



void
delay_ms (unsigned int length)
{
  int i;
  for (i = 0; i < length; i++)
    delay_1ms ();
}


