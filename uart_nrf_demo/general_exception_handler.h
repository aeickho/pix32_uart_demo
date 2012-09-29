#ifndef _general_exception_handler_H_
#define _general_exception_handler_H_
#include <p32xxxx.h>
#include <plib.h>

#define U_STAbits       U2STAbits
#define U_TXREG         U2TXREG


void __attribute__ ((nomips16)) _general_exception_handler (void);

#endif
