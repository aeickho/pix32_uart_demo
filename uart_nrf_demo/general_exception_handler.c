#include "general_exception_handler.h"


  // this function overrides the normal _weak_ generic handler
void __attribute__ ((nomips16)) _general_exception_handler (void)
{
  register unsigned int i;

  asm volatile ("mfc0 %0,$13":"=r" (_excep_code));
  asm volatile ("mfc0 %0,$14":"=r" (_excep_addr));

  _excep_code = (_excep_code & 0x0000007C) >> 2;

  unsigned int val = _excep_code;

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

  while (1)
    {
      ;
    }
}
