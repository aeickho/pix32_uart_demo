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
void
rftransfer_send (uint16_t size, uint8_t * data)
{
  uint8_t mac[5] = { 1, 2, 3, 2, 1 };
  struct NRF_CFG config = {
    .channel = 81,
    .txmac = "\x1\x2\x3\x2\x1",
    .nrmacs = 1,
    .mac0 = "\x1\x2\x3\x2\x1",
    .maclen = "\x20",
  };

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
  uint8_t outBuf[32];
  uint16_t cnt;
  uint16_t bigbuf[32];
  uint16_t bigbufcnt = 0;

  uint16_t old_cnt = 0;

  int c;

  /* Configure PB frequency and wait states */
  SYSTEMConfigPerformance (SystemClock ());
  UART2Init (SystemClock ());

  INTConfigureSystem (INT_SYSTEM_CONFIG_MULT_VECTOR);
  INTEnableInterrupts ();



  ANSELA = 0;
  ANSELB = 0;
  ANSELC = 0;




// LED2
  LATAbits.LATA10 = 0;		// LED2
  TRISAbits.TRISA10 = 0;



  UART2PutStr ("...........................\n\rhallo\r\n");
  UART2PutStr ("Welt\r\n");

  

  UART2PutStr ("nrf_init(),");
  // reset nRF 
   //   D4			37: RPC4/PMA4/RC4
  //RC4
  LATCbits.LATC4 = 0;		// RC4
  TRISCbits.TRISC4 = 0;

  LATCCLR = _LATC_LATC4_MASK;
  _delay_ms (10);
  LATCSET = _LATC_LATC4_MASK;


  nrf_init ();
  UART2PutStr ("done\n\r");

  

//  openbeaconSend ();
  UART2PutStr ("done\n\r");

  UART2PutStr ("nrf_config_set(),");
  config.nrmacs = 1;
  config.maclen[0] = 32;
  config.channel = 81;
  memcpy (config.mac0, "\x01\x02\x03\x02\x01", 5);
  nrf_config_set (&config);
  UART2PutStr ("\n\r");

  nrf_rcv_pkt_start ();

  do
    {
      int rcv = nrf_rcv_pkt_poll (32, buf);
      if (rcv != 0)
	{
	  int i;
	  uint16_t cnt;

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
//        UART2PutStr ("\n\r");

/*	  if (old_cnt != (cnt - 1))
	    {
	      ultoa (outBuf, cnt, 10);
	      UART2PutStr (outBuf);

	      UART2PutStr ("\n\r");
	      bigbufcnt = 0;
	    }
	  old_cnt = cnt;
*/
	}
    }
  while (1);

  return 0;
}
