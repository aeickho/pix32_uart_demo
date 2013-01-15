#include <p32xxxx.h>
#include <plib.h>
#include <stdint.h>

#include "myspi2.h"

#include "uart.h"
#include "general_exception_handler.h"


void
SPI2_UART2PutDbgStr (const char *buf)
{
#ifdef DEBUG_ON
  UART2PutStr (buf);
  UART2PutStr ("\r\n");
#endif
}




void
SPI2_init (void)
// Initialize pins for spi communication
{
  SPI2_UART2PutDbgStr (__func__);

// SPI2
  TRISBbits.TRISB5 = 0;		// SD02 as output
  RPB5R = 4;			// SDO2

  TRISBbits.TRISB13 = 1;	// SDI2 as input
  SDI2R = 3;			// RPB13;

  TRISBbits.TRISB15 = 0;	// SCK2 as output  (fixed pin)

// CS// CE
  TRISCbits.TRISC2 = 0;		// CSN as output
  TRISCbits.TRISC3 = 0;		// CE as output

  TRISAbits.TRISA1 = 1;

  SPI2BRG = 4;
  SPI2STATCLR = 1 << 6;		// clear SPIROV  
  SPI2CON = 0x8120;
}

uint8_t
SPI2_xmit (const uint8_t data)
{
  SPI2BUF = data;
  while (!SPI2STATbits.SPIRBF);
  return (SPI2BUF);
}

void
SPI2_transmit (uint8_t * data, const uint32_t len)
{
  int i;

  for (i = 0; i < len; i++)
    data[i]=SPI2_xmit (data[i]);
}
