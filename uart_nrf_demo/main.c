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

#define U_STAbits       U2STAbits
#define U_TXREG         U2TXREG




const char mainMenu[] =
  { "Welcome to PIC32 UART Peripheral Library Demo +  nRF24L01+ !\r\n" };


#define MAXPACKET   32
void
rftransfer_send (uint16_t size, uint8_t * data)
{

  UART2PutStr ("rftransfer_send\n\r");


  static struct NRF_CFG oldconfig;
  nrf_config_get (&oldconfig);

  nrf_set_channel (81);
  nrf_set_strength (3);
  const uint8_t macx[5] = { 1, 2, 3, 2, 1 };
  nrf_set_tx_mac (sizeof (macx), macx);



  uint8_t buf[MAXPACKET];
  buf[0] = 'L';
  buf[1] = size >> 8;
  buf[2] = size & 0xFF;

//  uint16_t rand = getRandom () & 0xFFFF;
  uint16_t rand = 0x5555;

  buf[3] = rand >> 8;
  buf[4] = rand & 0xFF;

//  nrf_config_set (&config);
  nrf_snd_pkt_crc (32, buf);	//setup packet
  _delay_ms (20);
  uint16_t index = 0;
  uint8_t i;
  uint16_t crc = crc16 (data, size);

  while (size)
    {
      buf[0] = 'D';
      buf[1] = index >> 8;
      buf[2] = index & 0xFF;
      buf[3] = rand >> 8;
      buf[4] = rand & 0xFF;
      for (i = 5; i < MAXPACKET - 2 && size > 0; i++, size--)
	{
	  buf[i] = *data++;
	}
      index++;
//      nrf_config_set (&config);
      nrf_snd_pkt_crc (32, buf);	//data packet
      _delay_ms (20);
    }

  buf[0] = 'C';
  buf[1] = crc >> 8;
  buf[2] = crc & 0xFF;
  buf[3] = rand >> 8;
  buf[4] = rand & 0xFF;
//  nrf_config_set (&config);
  nrf_snd_pkt_crc (32, buf);	//setup packet
  _delay_ms (20);

  nrf_config_set (&oldconfig);

}



int
main (void)
{
  struct NRF_CFG config;
  uint8_t buf[32];
  char outBuf[32];
  uint16_t cnt;
  uint8_t tmpBuf[4];


  int c;

  char strIn[20];

  const char str_sendblock[] = "sendblock";

  uint8_t *blockbuff;



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



/*
while (1)
 {
 c=UART1ReadChar();
 if (c>0) UART2SendChar((char) c);

 c=UART2ReadChar();
 if (c>0) UART1SendChar((char) c);
 }

*/
  UART2PutStr ("nrf_init(),");
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
  nrf_config_set (&config);
  UART2PutStr ("done\n\r");

  struct NRF_CFG oldconfig;

  struct NRF_CFG config1 = {
    .channel = 81,
    .txmac = "\x1\x2\x3\x2\x1",
    .nrmacs = 1,
    .mac0 = "\x1\x2\x3\x2\x1",
    .maclen = "\x20",
  };

  nrf_config_get (&oldconfig);
  UART2PutStr ("config_get_done\n\r");

  nrf_config_set (&config1);
  UART2PutStr ("config_set_done\n\r");


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
/*
      ultoa (outBuf, cnt, 10);
      UART2PutStr ("\n\r");
      UART2PutStr ("cnt: ");
      UART2PutStr (outBuf);
      UART2PutStr ("\n\r");
*/
      nrf_snd_pkt_crc (32, buf);
      _delay_ms (10);
    }
  while (1);
  return 0;
}
