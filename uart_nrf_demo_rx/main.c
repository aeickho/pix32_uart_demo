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
#include "base128.h"
#include "diskio.h"

#define SystemClock()                        (40000000ul)
#define GetPeripheralClock()            (SystemClock()/(1 << OSCCONbits.PBDIV))
#define GetInstructionClock()           (SystemClock())


#define MAXPACKET   32


struct rx_data
{
  uint32_t rx_timestamp;
  uint8_t serialnumber;
  uint8_t type;
  uint8_t restart_cnt;
  uint32_t packet_cnt;
  uint8_t sensorid;
  uint8_t ageing;
  uint8_t strength;
  uint8_t data[14];

};

#define MAX_RX_DATA 20

int
main (void)
{
  struct NRF_CFG config;
  uint8_t buf[32];
  uint8_t outBuf[128];
  uint32_t ret;
  uint32_t seq_nr = 0;
  struct rx_data rx_data_buffer[MAX_RX_DATA];
//  uint8_t diskbuf[512];


  uint32_t i;

  /* Configure PB frequency and wait states */
  SYSTEMConfigPerformance (SystemClock ());

  portsetup ();
  UART1Init (SystemClock ());
  UART2Init (SystemClock ());

  INTConfigureSystem (INT_SYSTEM_CONFIG_MULT_VECTOR);
  INTEnableInterrupts ();
  UART1PutStr ("\n\rhallo\r\n");
  UART2PutStr ("\n\rhallo\r\n");




  UART2PutStr ("nrf_init(),");
  nrf_init ();
  UART2PutStr ("done\n\r");

  memset (rx_data_buffer, 0, sizeof (rx_data_buffer));


/*
  UART2PutStr ("spi_sd_init,");
  SPI1_init ();
  UART2PutStr ("done\n\r");

  unsigned int stat = disk_initialize ();

  UART2PutStr ("disk_init: ");
  UART2PutHex (stat);
  UART2PutStr ("\r\n");



  disk_ioctl (0, GET_SECTOR_COUNT, &ret);
  UART2PutStr ("GET_SECTOR_COUNT: ");
  UART2PutHex (ret);
  UART2PutStr ("\r\n");


  disk_read (0, diskbuf, 0, 1);


  i =
    diskbuf[0x1c9] << 24 | diskbuf[0x1c8] << 16 | diskbuf[0x1c7] << 8 |
    diskbuf[0x1c6];
  UART2PutHex (i);
  UART2PutStr ("\n\r");

  i =
    diskbuf[0x1cD] << 24 | diskbuf[0x1cC] << 16 | diskbuf[0x1cB] << 8 |
    diskbuf[0x1cA];
  UART2PutHex (i);
  UART2PutStr ("\n\r");

*/

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
	  char cbuf[100];
	  uint32_t tmpspace[10];
	  uint32_t packet_cnt;
	  uint8_t storeflag = 0;
	  *tmpspace = (int) seq_nr;

/*	  UART1SendChar (0x01);
	  to_base128 ((uint8_t *) tmpspace, outBuf);
	  UART1Send (outBuf, 8);
	  to_base128n (buf, outBuf, 5);
	  UART1Send (outBuf, 40);
*/
	  ultoa (cbuf, (int) seq_nr, 10);
	  UART2PutStr (cbuf);
	  UART2PutStr (" : ");

	  for (i = 0; i < 32; i++)
	    {
	      ultoa (cbuf, buf[i], 16);
	      UART2PutStr (cbuf);
	      UART2PutStr (" ");
	    }
	  UART2PutStr ("\r\n");
	  seq_nr++;

	  packet_cnt = buf[3] << 24 | buf[4] << 16 | buf[5] << 8 | buf[6];
	  UART2PutStr ("packet_cnt: ");
	  ultoa (cbuf, (int) packet_cnt, 10);
	  UART2PutStr (cbuf);
	  UART2PutStr ("\r\n");

	  for (i = 0; i < MAX_RX_DATA; i++)
	    {
	      if (rx_data_buffer[i].serialnumber == buf[0])
		if (rx_data_buffer[i].type == buf[1])
		  if (rx_data_buffer[i].sensorid == buf[7])
		    if (rx_data_buffer[i].ageing >= buf[8])
		      if (rx_data_buffer[i].packet_cnt < packet_cnt
			  || rx_data_buffer[i].restart_cnt < buf[2]
			  || rx_data_buffer[i].strength > buf[9])
			{
			  rx_data_buffer[i].ageing = buf[8];
			  rx_data_buffer[i].packet_cnt = packet_cnt;
			  rx_data_buffer[i].restart_cnt = buf[2];
			  rx_data_buffer[i].strength = buf[9];
			  memcpy (&rx_data_buffer[i].data, &buf[16], 16 - 2);
			  storeflag = i;
			}
	    }
	  if (storeflag == 0)
	    {
	      for (i = 0; i < MAX_RX_DATA; i++)
		{
		  if (rx_data_buffer[i].serialnumber == 0)
		    {
		      rx_data_buffer[i].serialnumber = buf[0];
		      rx_data_buffer[i].type = buf[1];
		      rx_data_buffer[i].sensorid = buf[7];
		      rx_data_buffer[i].ageing = buf[8];
		      rx_data_buffer[i].packet_cnt = packet_cnt;
		      rx_data_buffer[i].restart_cnt = buf[2];
		      rx_data_buffer[i].strength = buf[9];
		      rx_data_buffer[i].ageing = buf[8];
		      rx_data_buffer[i].packet_cnt = packet_cnt;
		      rx_data_buffer[i].restart_cnt = buf[2];
		      rx_data_buffer[i].strength = buf[9];
		      memcpy (&rx_data_buffer[i].data, &buf[16], 16 - 2);
		      storeflag = i;
		    }
		}
	      if (storeflag == 0)
		UART2PutStr ("nsp\r\n");
	    }

	  UART1SendChar (0x0c);
	  for (i = 0; i < MAX_RX_DATA; i++)
	    {
	      int ii;
	      if (storeflag == i)
		UART1PutStr ("*");
	      else
		UART1PutStr (" ");


	      ultoa (cbuf, (int) rx_data_buffer[i].serialnumber, 10);
	      UART1PutStr (cbuf);
	      UART1PutStr (" ");
	      ultoa (cbuf, (int) rx_data_buffer[i].type, 10);
	      UART1PutStr (cbuf);
	      UART1PutStr (" ");
	      ultoa (cbuf, (int) rx_data_buffer[i].sensorid, 10);
	      UART1PutStr (cbuf);
	      UART1PutStr (" ");
	      ultoa (cbuf, (int) rx_data_buffer[i].ageing, 10);
	      UART1PutStr (cbuf);
	      UART1PutStr (" ");
	      ultoa (cbuf, (int) rx_data_buffer[i].packet_cnt, 10);
	      UART1PutStr (cbuf);
	      UART1PutStr (" ");
	      ultoa (cbuf, (int) rx_data_buffer[i].restart_cnt, 10);
	      UART1PutStr (cbuf);
	      UART1PutStr (" ");
	      ultoa (cbuf, (int) rx_data_buffer[i].strength, 10);
	      UART1PutStr (cbuf);
	      UART1PutStr (" ");
	      ultoa (cbuf, (int) rx_data_buffer[i].ageing, 10);
	      UART1PutStr (cbuf);
	      UART1PutStr (" ");
	      ultoa (cbuf, (int) rx_data_buffer[i].packet_cnt, 10);
	      UART1PutStr (cbuf);
	      UART1PutStr (" ");
	      ultoa (cbuf, (int) rx_data_buffer[i].restart_cnt, 10);
	      UART1PutStr (cbuf);
	      UART1PutStr (" ");
	      ultoa (cbuf, (int) rx_data_buffer[i].strength, 10);
	      UART1PutStr (cbuf);
	      UART1PutStr (" ");
	      for (ii = 0; ii < 16 - 2; ii++)
		{
		  ultoa (cbuf, (int) rx_data_buffer[i].data[ii], 10);
		  UART1PutStr (cbuf);
		  UART1PutStr (":");
		}
	      UART1PutStr ("\r\n");
	    }
	}



    }
  while (1);
  return 0;
}
