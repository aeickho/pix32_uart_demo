#ifndef PORTSETUP_H__
#define PORTSETUP_H__

#include <p32xxxx.h>
#include <plib.h>
#include <string.h>
#include <stdint.h>


void portsetup (void);

#define SystemClock()                        (40000000ul)
#define GetPeripheralClock()            (SystemClock()/(1 << OSCCONbits.PBDIV))
#define GetInstructionClock()           (SystemClock())



#endif
