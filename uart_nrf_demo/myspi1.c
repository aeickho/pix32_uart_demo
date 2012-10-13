#include <p32xxxx.h>
#include <plib.h>
#include <stdint.h>

#include "myspi.h"
#include "Pinguino.h"
#include "uart.h"
#include "general_exception_handler.h"


DmaChannel dma0TxChn = DMA_CHANNEL0;	// DMA channel to use for our example
DmaChannel dma3RxChn = DMA_CHANNEL3;	// DMA channel to use for our example

volatile int Dma0TxIntFlag;	// flag used in interrupts, signal that DMA transfer ended
volatile int Dma3RxIntFlag;	// flag used in interrupts, signal that DMA transfer ended

void SPI1_UART2PutDbgStr(const  char * buf)
{
#ifdef DEBUG_ON
  UART2PutStr(buf);
  UART2PutStr("\r\n");
#endif  
}
    

void
SPI1_init (void)
// Initialize pins for spi communication
{
SPI1_UART2PutDbgStr (__func__);

TRISAbits.TRISA7 = 0;
LATAbits.LATA7 = 1; // set CS high
             
                       
// set up MOSI/MISO pins
 SDI1R = 5; // MISO / RX
 RPA9R = 3; // MOSI / TX
 
                                                                      
SPI1BRG = 85; /* 232.56 kHz */
SPI1CON = 0x8120;

SPI1STATCLR = 1 << 6;		// clear SPIROV  


  DmaChnOpen (dma0TxChn, DMA_CHN_PRI3, DMA_OPEN_DEFAULT);
  DmaChnSetEventControl (dma0TxChn,
			 DMA_EV_START_IRQ_EN |
			 DMA_EV_START_IRQ (_SPI1_TX_IRQ));
  INTSetVectorPriority (INT_VECTOR_DMA (dma0TxChn), INT_PRIORITY_LEVEL_5);

  INTSetVectorSubPriority (INT_VECTOR_DMA (dma0TxChn),
			   INT_SUB_PRIORITY_LEVEL_3);

  DmaChnOpen (dma3RxChn, DMA_CHN_PRI2, DMA_OPEN_DEFAULT);
  DmaChnSetEventControl (dma3RxChn,
			 DMA_EV_START_IRQ_EN |
			 DMA_EV_START_IRQ (_SPI1_RX_IRQ));
  INTSetVectorPriority (INT_VECTOR_DMA (dma3RxChn), INT_PRIORITY_LEVEL_5);

  INTSetVectorSubPriority (INT_VECTOR_DMA (dma3RxChn),
			   INT_SUB_PRIORITY_LEVEL_3);
}

//  ultoa (outBuf, cnt, 10);
//  UART2PutStr ("\n\r");
//  UART2PutStr ("cnt: ");

// dma read 
void
SPI1_read (uint8_t * inBuf, uint8_t fillchar, uint8_t length)
{
  volatile int i;
 
  SPI1_UART2PutDbgStr (__func__);
  
  DmaChnSetTxfer (dma3RxChn, (void *) &SPI1BUF, inBuf, 1, length, 1);
  DmaChnSetEvEnableFlags (dma3RxChn, DMA_EV_BLOCK_DONE);
  Dma3RxIntFlag = 0;

   RPA9R = 0;
  if (fillchar == 0xff) 
    LATASET = _LATA_LATA9_MASK;
  else if (fillchar == 0)
    LATACLR = _LATA_LATA9_MASK;
  else
    _general_exception_handler ();	// darf nicht vorkommen, geht so aber noch nicht

  INTEnable (INT_SOURCE_DMA (dma3RxChn), INT_ENABLED);
  DmaChnEnable (dma3RxChn);

  Dma0TxIntFlag = 0;
 
  DmaChnSetTxfer (dma0TxChn, inBuf, (void *) &SPI1BUF, length, 1, 1);  // dummy data, not used SDO2 set as output
  DmaChnSetEvEnableFlags (dma0TxChn, DMA_EV_BLOCK_DONE);
  INTEnable (INT_SOURCE_DMA (dma0TxChn), INT_ENABLED);

  DmaChnStartTxfer (dma0TxChn, DMA_WAIT_NOT, 0);


  while ( (!Dma3RxIntFlag) &&  (!Dma0TxIntFlag));

 RPA9R = 3;

  i=SPI1BUF;			// Clear Input
    
  SPI1STATCLR = 1 << 6;		// clear SPIROV  
  SPI1CON = 0x8120;
}

void
SPI1_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len)
{
  volatile uint8_t out;
  volatile uint8_t i;
  volatile uint8_t dummy;
  for (i = 0; i < len; i++)
    {
      out = dataout[i];
      SPI1BUF = out;
      while (!SPI1STATbits.SPIRBF);
      dummy = SPI1BUF;
      datain[i] = dummy;
    }
}



uint8_t
SPI1_fast_shift (uint8_t data)
{
  SPI1BUF = data;
  while (!SPI1STATbits.SPIRBF);
  return SPI1BUF;
}


void
SPI1_transmit_sync (const uint8_t * dataout, uint8_t len)
{
  volatile uint8_t i;

  if (len > 10)
    {
      DmaChnSetTxfer (dma0TxChn, dataout, (void *) &SPI1BUF, len, 1, 1);
      DmaChnSetEvEnableFlags (dma0TxChn, DMA_EV_BLOCK_DONE);	// enable the transfer done interrupt, when all buffer transferred

      Dma0TxIntFlag = 0;

      INTEnable (INT_SOURCE_DMA (dma0TxChn), INT_ENABLED);

      DmaChnStartTxfer (dma0TxChn, DMA_WAIT_NOT, 0);
      while (!Dma0TxIntFlag);

      i = SPI1BUF;		// sonst geht nichts ;-)
      SPI1STATCLR = 1 << 6;	// clear SPIROV  
    }
  else
    {

      for (i = 0; i < len; i++)
	{
	  volatile char dummy;
	  SPI1BUF = dataout[i];
	  while (!SPI1STATbits.SPIRBF);
	  dummy = SPI1BUF;
	}
    }
}

void
  __attribute__ ((nomips16, interrupt (ipl5),
		  vector (_DMA1_VECTOR))) DmaHandler0 (void)
{
  int evFlags;			// event flags when getting the interrupt

  INTClearFlag (INT_SOURCE_DMA (DMA_CHANNEL0));	// acknowledge the INT controller, we're servicing int

  evFlags = DmaChnGetEvFlags (DMA_CHANNEL0);	// get the event flags

  if (evFlags & DMA_EV_BLOCK_DONE)
    {				// just a sanity check. we enabled just the DMA_EV_BLOCK_DONE transfer done interrupt
      Dma0TxIntFlag = 1;
      DmaChnClrEvFlags (DMA_CHANNEL0, DMA_EV_BLOCK_DONE);
      INTEnable (INT_SOURCE_DMA (DMA_CHANNEL0), INT_DISABLED);	///   verschoben
    }
}


void
  __attribute__ ((nomips16, interrupt (ipl5),
		  vector (_DMA2_VECTOR))) DmaHandler3 (void)
{
  int evFlags;			// event flags when getting the interrupt
  INTClearFlag (INT_SOURCE_DMA (DMA_CHANNEL3));	// acknowledge the INT controller, we're servicing int

  evFlags = DmaChnGetEvFlags (DMA_CHANNEL3);	// get the event flags

  if (evFlags & DMA_EV_BLOCK_DONE)
    {				// just a sanity check. we enabled just the DMA_EV_BLOCK_DONE transfer done interrupt
      Dma3RxIntFlag = 1;
      DmaChnClrEvFlags (DMA_CHANNEL3, DMA_EV_BLOCK_DONE);
      INTEnable (INT_SOURCE_DMA (DMA_CHANNEL3), INT_DISABLED);	///   verschoben
    }
}
