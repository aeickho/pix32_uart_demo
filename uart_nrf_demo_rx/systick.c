#include <p32xxxx.h>
#include <plib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "systick.h"

volatile uint64_t systick;

/* high priority interrupt as a 100 usec systick counter */
void
  __attribute__ ((nomips16, interrupt (ipl5),
		  vector (_TIMER_3_VECTOR))) systick_interrupt (void)
{
  systick++;
  IFS0bits.T3IF = 0;		/* reset interrupt flag */
}

void
systick_init ()
{
  /* set up timer 2 for life indicator */
  PR3 = 50;
  T3CON = 0x8030;		// enable, prescaler=1/8 -> 5 MHz
  IPC3bits.T3IP = 5;
  IEC0bits.T3IE = 1;
  systick=0;
}

unsigned long long __attribute__ ((nomips16))
 get_tics_i (void)
{
  unsigned int status = 0;
  unsigned long long tics;

  asm volatile ("di    %0":"=r" (status));
  tics = systick;
  asm volatile ("ei    %0":"=r" (status));
return (tics);

}

uint64_t get_systics (void)
{
return get_tics_i();
}
