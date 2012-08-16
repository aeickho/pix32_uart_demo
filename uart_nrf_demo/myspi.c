#include <p32xxxx.h>
#include <plib.h>
#include <stdint.h>

#include "myspi.h"
#include "Pinguino.h"

void
spi_init (void)
// Initialize pins for spi communication
{
  ;
}

void
spi_transfer_sync (uint8_t * dataout, uint8_t * datain,
		   uint8_t len)
{
  uint8_t outBuf[100];
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
spi_transmit_sync (const uint8_t * dataout, uint8_t len)
{
  uint8_t i;
  for (i = 0; i < len; i++)
    {
      SPI2BUF = dataout[i];
      while (!SPI2STATbits.SPIRBF);
      char dummy = SPI2BUF;
    }
}

uint8_t
spi_fast_shift (uint8_t data)
{
  SPI2BUF = data;
  while (!SPI2STATbits.SPIRBF);
  return SPI2BUF;
}
