/*
	Example "blinkenlights" program for the UBW32 and C32 compiler.
	Demonstrates software PWM, use of floating-point library, etc.

	IMPORTANT: file 'procdefs.ld' absolutely must Must MUST be present
	in the working directory when compiling code for the UBW32!  Failure
	to do so will almost certainly result in your UBW32 getting 'bricked'
	and requiring re-flashing the bootloader (which, if you don't have a
	PICkit 2 or similar PIC programmer, you're screwed).
	YOU HAVE BEEN WARNED.

	2/19/2009 - Phillip Burgess - pburgess@dslextreme.com
*/

#include <p32xxxx.h>
#include <plib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Pinguino.h"
#include "nrf24l01p.h"
#include "myspi.h"
#include "basic.h"
#include "byteorder.h"
#include "uart.h"
#include "portsetup.h"
//#include "hw_spi1.h"
#include "diskio.h"

#define SystemClock()                        (40000000ul)
#define GetPeripheralClock()            (SystemClock()/(1 << OSCCONbits.PBDIV))
#define GetInstructionClock()           (SystemClock())


#define MAXPACKET   32


int
main (void)
{
  struct NRF_CFG config;
  uint8_t buf[32];
  char   outBuf[32];
  uint16_t cnt;
  uint16_t bigbuf[32];
  uint16_t bigbufcnt = 0;
  uint32_t ret;
  uint16_t old_cnt = 0;

  uint8_t diskbuf[512];
 
  uint32_t * p;
   
  unsigned int i;
   char c;
  
  /* Configure PB frequency and wait states */
  SYSTEMConfigPerformance (SystemClock ());

  portsetup();
  UART1Init (SystemClock ());
  UART2Init (SystemClock ());

  INTConfigureSystem (INT_SYSTEM_CONFIG_MULT_VECTOR);
  INTEnableInterrupts ();
  UART1PutStr ("\n\rhallo\r\n");
  UART2PutStr ("\n\rhallo\r\n");




  UART2PutStr ("nrf_init(),");
  nrf_init ();
  UART2PutStr ("done\n\r");


  UART2PutStr ("spi_sd_init,");
  SPI1_init();
  UART2PutStr ("done\n\r");

  int counter = 0;
  unsigned int stat = disk_initialize ();
      
  UART2PutStr ("disk_init: ");
  UART2PutHex (stat);
  UART2PutStr ("\r\n");

                    

  disk_ioctl (0, GET_SECTOR_COUNT, &ret);
  UART2PutStr ("GET_SECTOR_COUNT: ");
  UART2PutHex (ret);
  UART2PutStr("\r\n");

  
  disk_read(0, diskbuf, 0, 1);

  
  i = diskbuf [ 0x1c9 ] << 24 |  diskbuf [ 0x1c8 ] << 16 |  diskbuf [ 0x1c7 ] << 8  |  diskbuf [ 0x1c6 ]; 
  UART2PutHex (i);
  UART2PutStr("\n\r");

  i = diskbuf [ 0x1cD ] << 24 |  diskbuf [ 0x1cC ] << 16 |  diskbuf [ 0x1cB ] << 8  |  diskbuf [ 0x1cA ]; 
  UART2PutHex (i);
  UART2PutStr("\n\r");

  UART2PutStr ("nrf_config_set(),");
  config.nrmacs = 1;
  config.maclen[0] = 32;
  config.channel = 81;
  memcpy (config.mac0, "\x01\x02\x03\x02\x01", 5);
  nrf_config_set (&config);
  UART2PutStr ("done\n\r");

  nrf_rcv_pkt_start ();

  do
    {
      int rcv = nrf_rcv_pkt_poll (32, buf);
      if (rcv == 32)
	{
	  int i;
	  uint16_t cnt;

	  
//	  76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210 76543210
//	  65432106 54321065 43210654 32106543 21065432 10654321 06543210 
//        
//        7654321 0765432 1076543 2107654 3210765 4321076 5432107 6543210	  
//  7 zu  8 1x	  
// 14 zu 16 2x
// 28 zu 32 4x
// 35 zu 40 5x



       

	  cnt = buf[2] << 8 | buf[3];


	  UART2PutStr ("\r\n");
	  UART2PutStr ("recv: ");
	  ultoa (outBuf, rcv, 10);

	  UART2PutStr (outBuf);
          UART2PutStr (" ");

	  ultoa (outBuf, old_cnt++, 10);
	  UART2PutStr (outBuf);
          UART2PutStr (" ");
 
	  ultoa (outBuf, cnt, 10);
	  UART2PutStr (outBuf);
	}
    }
  while (1);

  return 0;
}
