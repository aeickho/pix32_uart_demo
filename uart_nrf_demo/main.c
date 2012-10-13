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
#include "openbeacon.h"
#include "myspi.h"
#include "basic.h"
#include "byteorder.h"
#include "uart.h"

#define SystemClock()                        (40000000ul)
#define GetPeripheralClock()            (SystemClock()/(1 << OSCCONbits.PBDIV))
#define GetInstructionClock()           (SystemClock())


#define MAXPACKET   32


int
main (void)
{
  char outBuf[32];
  char strIn[20];
  char str_sendblock[] = "sendblock";
  int c;
  struct NRF_CFG config;
  uint16_t cnt;
  uint8_t *blockbuff;
  uint8_t buf[32],tmpBuf[4];

  /* Configure PB frequency and wait states */
  SYSTEMConfigPerformance (SystemClock ());
  UART1Init (SystemClock ());
  UART2Init (SystemClock ());

  INTConfigureSystem (INT_SYSTEM_CONFIG_MULT_VECTOR);
  INTEnableInterrupts ();

  ANSELA = 0;
  ANSELB = 0;
  ANSELC = 0;

// LED2
  LATAbits.LATA10 = 0;		// LED2
  TRISAbits.TRISA10 = 0;

  blockbuff = malloc (512);
  if (blockbuff == NULL)
    {
      UART2PutStr ("cant' malloc\r\n");
      while (1);
    }

  UART1PutStr
    (".............................................................................hallo\r\n");
  UART1PutStr ("UART1 Welt\r\n");


  UART2PutStr
    (".............................................................................hallo\r\n");
  UART2PutStr ("UART2 Welt\r\n");

  // try to reset nrf chip

  UART2PutStr ("nrf_init(),");

  // reset nRF 
  //RC4
  LATCbits.LATC4 = 0;          // RC4
  TRISCbits.TRISC4 = 0;
  
  LATBCLR = _LATC_LATC4_MASK;
  _delay_ms (10);
  LATBSET = _LATC_LATC4_MASK;  
     

  nrf_init ();


  UART2PutStr ("done\n\r");

  UART2PutStr ("openbeaconSend(),");
  openbeaconSend ();
  UART2PutStr ("done\n\r");

  UART2PutStr ("nrf_config_set(),");
  config.nrmacs = 1;
  config.maclen[0] = 32;
  config.channel = 81;
  ultoa (outBuf, config.channel, 10);
  UART2PutStr ("cannel: ,");
  UART2PutStr (outBuf);
  memcpy (config.mac0, "\x01\x02\x03\x02\x01", 5);
  memcpy (config.txmac, "\x01\x02\x03\x02\x01", 5);
  nrf_config_set (&config);
  UART2PutStr ("done\n\r");

  buf[0] = 32;

  for (c = 1; c < 32; c++)
    buf[c] = c;

  UART2PutStr ("send:\n\r");
  cnt = 0;


  do
    {
      c = UART1ReadChar ();
      if (c > 0)
	{
	  int i;
	  UART2SendChar ((char) c);
	  if ((char) c == '\n')
	    {
	      UART2PutStr ("\n\r--");
	      UART2PutStr (strIn);
	      UART2PutStr ("--\n\r");
	      if (!strncmp (strIn, str_sendblock, strlen (str_sendblock)))
		{
		  uint32_t sektor;
		  uint16_t sender_crc16, calc_crc16;

		  UART2PutStr (">sendblock\n\r");
		  UART1Read (tmpBuf, 4);
		  sektor =
		    tmpBuf[3] << 24 | tmpBuf[2] << 16 | tmpBuf[1] << 8 |
		    tmpBuf[0];

		  UART1Read (tmpBuf, 2);
		  sender_crc16 = tmpBuf[1] << 8 | tmpBuf[0];

		  ultoa (outBuf, sender_crc16, 10);
		  UART2PutStr ("\n\r");
		  UART2PutStr ("sender crc16: ");
		  UART2PutStr (outBuf);
		  UART2PutStr ("\n\r");

		  UART1Read (blockbuff, 512);

		  calc_crc16 = crc16 (blockbuff, 512);
		  ultoa (outBuf, calc_crc16, 10);
		  UART2PutStr ("\n\r");
		  UART2PutStr ("calc crc16: ");
		  UART2PutStr (outBuf);
		  UART2PutStr ("\n\r");



//                UART2Send (blockbuff, 512);
		  UART2PutStr ("done\n\r");

		}

	      strIn[0] = '\0';
	      UART1PutStr ("ready\n\r");
	    }
	  else if (strlen (strIn) < 17)
	    {
	      for (i = 0; i < (17 - 1) && strIn[i] != 0; i++);
	      strIn[i++] = (char) c;
	      strIn[i++] = '\0';
	    }

	}

      cnt++;
      buf[2] = cnt >> 8;
      buf[3] = cnt & 0xff;

      ultoa (outBuf, cnt, 10);
      UART2PutStr ("\n\r");
      UART2PutStr ("cnt: ");
      UART2PutStr (outBuf);
      UART2PutStr ("\n\r");

      nrf_snd_pkt_crc (32, buf);
      _delay_ms (10);
    }
  while (1);
  return 0;
}
