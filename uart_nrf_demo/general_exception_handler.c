#include "general_exception_handler.h"

static enum
{
  EXCEP_IRQ = 0,		// interrupt
  EXCEP_AdEL = 4,		// address error exception (load or ifetch)
  EXCEP_AdES,			// address error exception (store)
  EXCEP_IBE,			// bus error (ifetch)
  EXCEP_DBE,			// bus error (load/store)
  EXCEP_Sys,			// syscall
  EXCEP_Bp,			// breakpoint
  EXCEP_RI,			// reserved instruction
  EXCEP_CpU,			// coprocessor unusable
  EXCEP_Overflow,		// arithmetic overflow
  EXCEP_Trap,			// trap (possible divide by zero)
  EXCEP_IS1 = 16,		// implementation specfic 1
  EXCEP_CEU,			// CorExtend Unuseable
  EXCEP_C2E			// coprocessor 2
} _excep_code;

							// static unsigned int _epc_code;
static unsigned int _excep_addr;


  // this function overrides the normal _weak_ generic handler
void __attribute__ ((nomips16)) _general_exception_handler (void)
{
  register unsigned int i;

  asm volatile ("mfc0 %0,$13":"=r" (_excep_code));
  asm volatile ("mfc0 %0,$14":"=r" (_excep_addr));

  _excep_code = (_excep_code & 0x0000007C) >> 2;

  unsigned int val = _excep_code;

//  while (1)
    {
      while (U_STAbits.UTXBF);	// wait when buffer is full
      U_TXREG = 'E';
      while (U_STAbits.UTXBF);	// wait when buffer is full
      U_TXREG = ':';
      for (i = 0; i < 8; i++)
	{
	  int bla = ((val & 0xF0000000) >> 28);
	  while (U_STAbits.UTXBF);

	  if (bla > 9)
	    U_TXREG = (('A' - 10) + bla);
	  else
	    U_TXREG = ('0' + bla);
	  val <<= 4;
	}

      while (U_STAbits.UTXBF);	// wait when buffer is full
      U_TXREG = ' ';

      val = _excep_addr;
      for (i = 0; i < 8; i++)
	{
	  int bla = ((val & 0xF0000000) >> 28);
	  while (U_STAbits.UTXBF);

	  if (bla > 9)
	    U_TXREG = (('A' - 10) + bla);
	  else
	    U_TXREG = ('0' + bla);
	  val <<= 4;
	}

      while (U_STAbits.UTXBF);	// wait when buffer is full
      U_TXREG = '\r';
      while (U_STAbits.UTXBF);	// wait when buffer is full
      U_TXREG = '\n';
     while(1);
    }
}
