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
// (D10) PGEC3/VREF-/CVREF-/AN1/RPA1/CTED2/PMD6/RA1

  TRISAbits.TRISA1 = 1;


  CS_HIGH ();			// NO SPI Chip Select
  CE_LOW ();			// NO Chip Enable Activates RX or TX mode

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
      while (!SPI2STATbits.SPIRBF);
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
  UART2PutStr ("TF 1\r\n");
  
  for (i = 0; i < len; i++)
    {
      out = dataout[i];
      SPI2BUF = out;
      while (!SPI2STATbits.SPIRBF);
      char dummy = SPI2BUF;
      datain[i] = dummy;
    }
  UART2PutStr ("TF 2\r\n");

}

volatile int DmaTxIntFlag;		// flag used in interrupts, signal that DMA transfer ended

void
__ISR (_DMA1_VECTOR, ipl5)
DmaHandler1 (void)
{
  int evFlags;			// event flags when getting the interrupt

  INTClearFlag (INT_SOURCE_DMA (DMA_CHANNEL1));	// acknowledge the INT controller, we're servicing int

  evFlags = DmaChnGetEvFlags (DMA_CHANNEL1);	// get the event flags

  if (evFlags & DMA_EV_BLOCK_DONE)
    {				// just a sanity check. we enabled just the DMA_EV_BLOCK_DONE transfer done interrupt
      DmaTxIntFlag = 1;
      DmaChnClrEvFlags (DMA_CHANNEL1, DMA_EV_BLOCK_DONE);
    }
}

SPI2_transmit_sync (const uint8_t * dataout, uint8_t len)
{
  uint8_t i;
  char buf[10];

               
  if (len > 5)
    {
   ultoa (buf, len, 10);
        UART2PutStr ("TS len: ");
               UART2PutStr (buf);
      UART2PutStr ("\r\n");

      UART2PutStr ("\r\n");
      UART2PutStr ("TS 1\r\n");

      DmaChannel dmaTxChn = DMA_CHANNEL1;	// DMA channel to use for our example
      // NOTE: the DMA ISR setting has to match the channel number
      SpiChannel spiTxChn = SPI_CHANNEL2;	// the transmitting SPI channel to use in our example

//SpiChnOpen(spiTxChn, SPI_OPEN_MSTEN|SPI_OPEN_SMP_END|SPI_OPEN_MODE8, 4);


//      INTDisableInterrupts ();

      UART2PutStr ("TS 1.0\r\n");
      DmaChnOpen (dmaTxChn, DMA_CHN_PRI2, DMA_OPEN_DEFAULT);

      UART2PutStr ("TS 1.1\r\n");
      DmaChnSetEventControl (dmaTxChn,
			     DMA_EV_START_IRQ_EN |
			     DMA_EV_START_IRQ (_SPI2_TX_IRQ));
      UART2PutStr ("TS 1.2\r\n");

      DmaChnSetTxfer (dmaTxChn, dataout, (void *) &SPI2BUF, len, 1, 1);
      UART2PutStr ("TS 1.3\r\n");
      DmaChnSetEvEnableFlags (dmaTxChn, DMA_EV_BLOCK_DONE);	// enable the transfer done interrupt, when all buffer transferred
      UART2PutStr ("TS 1.4\r\n");


      INTSetVectorPriority (INT_VECTOR_DMA (dmaTxChn), INT_PRIORITY_LEVEL_5);
      UART2PutStr ("TS 1.5\r\n");

      INTSetVectorSubPriority (INT_VECTOR_DMA (dmaTxChn),
			       INT_SUB_PRIORITY_LEVEL_3);

      UART2PutStr ("TS 2\r\n");
      DmaTxIntFlag = 0;		// clear the interrupt flag we're  waiting on

      INTEnable (INT_SOURCE_DMA (dmaTxChn), INT_ENABLED);
//      INTEnableInterrupts ();
      DmaChnStartTxfer (dmaTxChn, DMA_WAIT_NOT, 0);
      UART2PutStr ("TS 3\r\n");
      // wait for the transfer to complete
      // In a real application you can do some other stuff while the DMA transfer is taking place
      while (!DmaTxIntFlag)
	{
      UART2PutStr ("TS 4\r\n");
	}

        while (!SPI2STATbits.SPIRBF)
        
        {
              UART2PutStr ("TS 4.1\r\n");
                      }
              int ret;      
 
UART2PutStr ("TS 5\r\n");

      INTEnable (INT_SOURCE_DMA (dmaTxChn), INT_DISABLED);
  SPI2BRG = 4;
  SPI2STATCLR = 1 << 6;		// clear SPIROV  
  SPI2CON = 0x8120;

UART2PutStr ("TS 6\r\n");


    }
  else
    {
      for (i = 0; i < len; i++)
	{
	  SPI2BUF = dataout[i];
	  while (!SPI2STATbits.SPIRBF);
	  char dummy = SPI2BUF;
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
