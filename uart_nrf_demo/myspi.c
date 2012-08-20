#include <p32xxxx.h>
#include <plib.h>
#include <stdint.h>

#include "myspi.h"
#include "Pinguino.h"

void
SPI2_init (void)
// Initialize pins for spi communication
{
// SPI2
  TRISBbits.TRISB5 = 0;		// SD02 as output
  RPB5R = 4;			// SDO2

  TRISBbits.TRISB13 = 1;	// SDI2 as input
  SDI2R = 3;			// RPB13;

  TRISBbits.TRISB15 = 0;	// SCK2 as output  (fixed pin)

// CS// CE
  TRISCbits.TRISC2 = 0;		// CSN as output
  TRISCbits.TRISC3 = 0;		// CE as output

  CS_HIGH ();			// NO SPI Chip Select
  CE_LOW ();			// NO Chip Enable Activates RX or TX mode

/*  SPI2CON = 0;
  SPI2CONbits.MSTEN = 1;
  SPI2CONbits.CKE = 1;
//  SPI2CONbits.SMP = 1; 
  SPI2BRG = 1;
  SPI2CON2 = 0;
  SPI2CONbits.ON = 1;
*/
/*
 OpenSPI2( SPI_MODE8_ON | MASTER_ENABLE_ON | SEC_PRESCAL_1_1 | PRI_PRESCAL_1_1 | FRAME_ENABLE_OFF | CLK_POL_ACTIVE_HIGH | ENABLE_SDO_PIN , SPI_ENABLE );
 SPI2BRG = 1;
*/
  SPI2BRG = 4;
  SPI2STATCLR = 1 << 6;		// clear SPIROV  
  SPI2CON = 0x8120;

  ;
}

void
SPI1_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len)
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
SPI1_transmit_sync (const uint8_t * dataout, uint8_t len)
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
SPI1_fast_shift (uint8_t data)
{
  SPI2BUF = data;
  while (!SPI2STATbits.SPIRBF);
  return SPI2BUF;
}



void
SPI2_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len)
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
SPI2_transmit_sync (const uint8_t * dataout, uint8_t len)
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
SPI2_fast_shift (uint8_t data)
{
  SPI2BUF = data;
  while (!SPI2STATbits.SPIRBF);
  return SPI2BUF;
}
