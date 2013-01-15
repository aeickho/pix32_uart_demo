#include <p32xxxx.h>
#include <plib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void
delay_7us (void)
{
  T1CON = 0x8030;
  PR1 = 0xffff;
  TMR1 = 0;
  while (TMR1 < 1);
}

void
delay_ms (unsigned int length)
{
  int i;
  for (i = 0; i < length; i++)
    {
      T1CON = 0x8030;
      PR1 = 0xffff;
      TMR1 = 0;
      while (TMR1 < 156);
    }
}
