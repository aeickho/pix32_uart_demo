#include <p32xxxx.h>
#include <plib.h>
#include <stdint.h>

#include "myspi.h"
#include "Pinguino.h"

DmaChannel dmaTxChn = DMA_CHANNEL1;	// DMA channel to use for our example
volatile int DmaTxIntFlag;	// flag used in interrupts, signal that DMA transfer ended



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
// (D10) PGEC3/VREF-/CVREF-/AN1/RPA1/CTED2/PMD6/RA1

  TRISAbits.TRISA1 = 1;


  CS_nRF_HIGH ();		// NO SPI Chip Select
  CE_nRF_LOW ();		// NO Chip Enable Activates RX or TX mode

  SPI2BRG = 4;
  SPI2STATCLR = 1 << 6;		// clear SPIROV  
  SPI2CON = 0x8120;

  DmaChnOpen (dmaTxChn, DMA_CHN_PRI2, DMA_OPEN_DEFAULT);
  DmaChnSetEventControl (dmaTxChn,
			 DMA_EV_START_IRQ_EN |
			 DMA_EV_START_IRQ (_SPI2_TX_IRQ));
  INTSetVectorPriority (INT_VECTOR_DMA (dmaTxChn), INT_PRIORITY_LEVEL_5);

  INTSetVectorSubPriority (INT_VECTOR_DMA (dmaTxChn),
			   INT_SUB_PRIORITY_LEVEL_3);


  ;
}

void
SPI2_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len)
{
  volatile uint8_t out;
  volatile uint8_t i;
  volatile uint8_t dummy;
  for (i = 0; i < len; i++)
    {

      out = dataout[i];
      SPI2BUF = out;
      while (!SPI2STATbits.SPIRBF);
      dummy = SPI2BUF;
      datain[i] = dummy;
    }
}

void
SPI2_transmit_sync (const uint8_t * dataout, uint8_t len)
{
  volatile uint8_t i;

  if (len > 10)
    {
      DmaChnSetTxfer (dmaTxChn, dataout, (void *) &SPI2BUF, len, 1, 1);
      DmaChnSetEvEnableFlags (dmaTxChn, DMA_EV_BLOCK_DONE);	// enable the transfer done interrupt, when all buffer transferred

      DmaTxIntFlag = 0;

      INTEnable (INT_SOURCE_DMA (dmaTxChn), INT_ENABLED);

      DmaChnStartTxfer (dmaTxChn, DMA_WAIT_NOT, 0);
      while (!DmaTxIntFlag);

      i = SPI2BUF;		// sonst geht nichts ;-)
      SPI2STATCLR = 1 << 6;	// clear SPIROV  
    }
  else
    {

      for (i = 0; i < len; i++)
	{
	  volatile char dummy;
	  SPI2BUF = dataout[i];
	  while (!SPI2STATbits.SPIRBF);
	  dummy = SPI2BUF;
	}
    }
}



uint8_t
SPI2_fast_shift (uint8_t data)
{
  SPI2BUF = data;
  while (!SPI2STATbits.SPIRBF);
  return SPI2BUF;
}


#define D_VECTOR        _DMA1_VECTOR
void
  __attribute__ ((nomips16, interrupt (ipl5),
		  vector (D_VECTOR))) DmaHandler1 (void)
{
  int evFlags;			// event flags when getting the interrupt

  INTClearFlag (INT_SOURCE_DMA (DMA_CHANNEL1));	// acknowledge the INT controller, we're servicing int


  evFlags = DmaChnGetEvFlags (DMA_CHANNEL1);	// get the event flags

  if (evFlags & DMA_EV_BLOCK_DONE)
    {				// just a sanity check. we enabled just the DMA_EV_BLOCK_DONE transfer done interrupt
      DmaTxIntFlag = 1;
      DmaChnClrEvFlags (DMA_CHANNEL1, DMA_EV_BLOCK_DONE);
      INTEnable (INT_SOURCE_DMA (DMA_CHANNEL1), INT_DISABLED);	///   verschoben
//      DmaChnDisable (DMA_CHANNEL1);
    }
}
