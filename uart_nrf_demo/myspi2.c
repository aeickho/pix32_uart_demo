#include <p32xxxx.h>
#include <plib.h>
#include <stdint.h>

#include "myspi.h"
#include "Pinguino.h"
#include "uart.h"
#include "general_exception_handler.h"


DmaChannel dmaTxChn = DMA_CHANNEL1;	// DMA channel to use for our example
DmaChannel dmaRxChn = DMA_CHANNEL2;	// DMA channel to use for our example

volatile int DmaTxIntFlag;	// flag used in interrupts, signal that DMA transfer ended
volatile int DmaRxIntFlag;	// flag used in interrupts, signal that DMA transfer ended

void SPI2_UART2PutDbgStr(const  char * buf)
{
#ifdef DEBUG_ON
  UART2PutStr(buf);
  UART2PutStr("\r\n");
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

//**************
  DmaChnOpen (dmaRxChn, DMA_CHN_PRI2, DMA_OPEN_DEFAULT);
  DmaChnSetEventControl (dmaRxChn,
			 DMA_EV_START_IRQ_EN |
			 DMA_EV_START_IRQ (_SPI2_RX_IRQ));
  INTSetVectorPriority (INT_VECTOR_DMA (dmaRxChn), INT_PRIORITY_LEVEL_5);

  INTSetVectorSubPriority (INT_VECTOR_DMA (dmaRxChn),
			   INT_SUB_PRIORITY_LEVEL_3);
//**************


  ;
}

//  ultoa (outBuf, cnt, 10);
//  UART2PutStr ("\n\r");
//  UART2PutStr ("cnt: ");

// dma read 
void
SPI2_read (uint8_t * inBuf, uint8_t fillchar, uint8_t length)
{
  volatile int i;
 
  SPI2_UART2PutDbgStr (__func__);
  
  DmaChnSetTxfer (dmaRxChn, (void *) &SPI2BUF, inBuf, 1, length, 1);
  DmaChnSetEvEnableFlags (dmaRxChn, DMA_EV_BLOCK_DONE);
  DmaRxIntFlag = 0;

  RPB5R = 0;
  if (fillchar == 0xff)
    LATBSET = _LATB_LATB5_MASK;
  else if (fillchar == 0)
    LATBCLR = _LATB_LATB5_MASK;
  else
    _general_exception_handler ();	// darf nicht vorkommen, geht so aber noch nicht

  INTEnable (INT_SOURCE_DMA (dmaRxChn), INT_ENABLED);
  DmaChnEnable (dmaRxChn);

  DmaTxIntFlag = 0;
 
  DmaChnSetTxfer (dmaTxChn, inBuf, (void *) &SPI2BUF, length, 1, 1);  // dummy data, not used SDO2 set as output
  DmaChnSetEvEnableFlags (dmaTxChn, DMA_EV_BLOCK_DONE);
  INTEnable (INT_SOURCE_DMA (dmaTxChn), INT_ENABLED);

  DmaChnStartTxfer (dmaTxChn, DMA_WAIT_NOT, 0);


  while ( (!DmaRxIntFlag) &&  (!DmaTxIntFlag));

  RPB5R = 4;			// SDO2

  i=SPI2BUF;			// Clear Input
    
  SPI2STATCLR = 1 << 6;		// clear SPIROV  
  SPI2CON = 0x8120;
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



uint8_t
SPI2_fast_shift (uint8_t data)
{
  SPI2BUF = data;
  while (!SPI2STATbits.SPIRBF);
  return SPI2BUF;
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

void
  __attribute__ ((nomips16, interrupt (ipl5),
		  vector (_DMA1_VECTOR))) DmaHandler1 (void)
{
  int evFlags;			// event flags when getting the interrupt

  INTClearFlag (INT_SOURCE_DMA (DMA_CHANNEL1));	// acknowledge the INT controller, we're servicing int

  evFlags = DmaChnGetEvFlags (DMA_CHANNEL1);	// get the event flags

  if (evFlags & DMA_EV_BLOCK_DONE)
    {				// just a sanity check. we enabled just the DMA_EV_BLOCK_DONE transfer done interrupt
      DmaTxIntFlag = 1;
      DmaChnClrEvFlags (DMA_CHANNEL1, DMA_EV_BLOCK_DONE);
      INTEnable (INT_SOURCE_DMA (DMA_CHANNEL1), INT_DISABLED);	///   verschoben
    }
}


void
  __attribute__ ((nomips16, interrupt (ipl5),
		  vector (_DMA2_VECTOR))) DmaHandler2 (void)
{
  int evFlags;			// event flags when getting the interrupt
//  SPI2_UART2PutDbgStr (__func__);
  INTClearFlag (INT_SOURCE_DMA (DMA_CHANNEL2));	// acknowledge the INT controller, we're servicing int

  evFlags = DmaChnGetEvFlags (DMA_CHANNEL2);	// get the event flags

  if (evFlags & DMA_EV_BLOCK_DONE)
    {				// just a sanity check. we enabled just the DMA_EV_BLOCK_DONE transfer done interrupt
      DmaRxIntFlag = 1;
      DmaChnClrEvFlags (DMA_CHANNEL2, DMA_EV_BLOCK_DONE);
      INTEnable (INT_SOURCE_DMA (DMA_CHANNEL2), INT_DISABLED);	///   verschoben
    }
}
