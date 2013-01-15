#include "portsetup.h"

void
portsetup (void)
{
  ANSELA = 0;
  ANSELB = 0;
  ANSELC = 0;

  LATAbits.LATA10 = 0;		// LED2
  TRISAbits.TRISA10 = 0;

}
