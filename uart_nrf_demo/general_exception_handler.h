#ifndef _general_exception_handler_H_
#define _general_exception_handler_H_
#include <p32xxxx.h>
#include <plib.h>

#define U_STAbits       U2STAbits
#define U_TXREG         U2TXREG



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

#endif
