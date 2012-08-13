#include <p32xxxx.h>
#include <plib.h>

#include "spi.h"
#include "Pinguino.h"
void
spi_init ()
// Initialize pins for spi communication
{
  ;
}

void
spi_transfer_sync (unsigned char *dataout, unsigned char *datain,
		   unsigned char len)
{
  char outBuf[100];
  int out;
  unsigned char i;
  for (i = 0; i < len; i++)
    {
      out = dataout[i];
      SPI2BUF = out;
      while (!SPI2STATbits.SPIRBF)
	{
	;
	}
     char dummy = SPI2BUF;
    
     datain[i] = dummy;

    }
}

void
spi_transmit_sync (const unsigned char *dataout, unsigned char len)
{
  unsigned char i;
  for (i = 0; i < len; i++)
    {
      SPI2BUF = dataout[i];
      while (!SPI2STATbits.SPIRBF);
      char dummy = SPI2BUF;
    }
}

unsigned char
spi_fast_shift (unsigned char data)
{
  SPI2BUF = data;
  while (!SPI2STATbits.SPIRBF);
  return SPI2BUF;
}
