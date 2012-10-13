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
  uint8_t buf[32],tmpBuf[4];
  
  

  portsetup();

  
  /* Configure PB frequency and wait states */
  SYSTEMConfigPerformance (SystemClock ());
//  UART1Init (SystemClock ());
  UART2Init (SystemClock ());

  INTConfigureSystem (INT_SYSTEM_CONFIG_MULT_VECTOR);
  INTEnableInterrupts ();




  UART2PutStr
    (".............................................................................hallo\r\n");
  UART2PutStr ("UART2 Welt\r\n");

  // try to reset nrf chip

  UART2PutStr ("nrf_init(),");
  nrf_init ();
  UART2PutStr ("done\n\r");

  UART2PutStr ("nrf_config_set(),");
  config.nrmacs = 1;
  config.maclen[0] = 32;
  config.channel = 81;
  ultoa (outBuf, config.channel, 10);
  UART2PutStr ("cannel: ,");
  UART2PutStr (outBuf);
  memcpy (config.mac0, "\x01\x02\x03\x02\x01", 5);
  memcpy (config.txmac, "\x1\x2\x3\x2\x1",5);
  nrf_config_set (&config);
  UART2PutStr ("done\n\r");


  buf[0] = 32;

  for (c = 1; c < 32; c++)
    buf[c] = c;

  UART2PutStr ("send:\n\r");
  cnt = 0;


  do
    {
      cnt++;
      buf[2] = cnt >> 8;
      buf[3] = cnt & 0xff;

      ultoa (outBuf, cnt, 10);
      UART2PutStr ("\n\r");
      UART2PutStr ("cnt: ");
      UART2PutStr (outBuf);
      UART2PutStr ("\n\r");

      nrf_snd_pkt_crc (32, buf);
      delay_ms (100);
    }
  while (1);
  return 0;
}
