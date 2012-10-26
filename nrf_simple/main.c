#include <p32xxxx.h>
#include <plib.h>
#include <stdint.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "nrf.h"
#include "myspi2.h"
#include "uart.h"
#include "portsetup.h"
#include "basic.h"
#include "delay.h"

#include "nrf_x.h"

#define WORDS_PER_FRAGMENT   6
#define BYTES_PER_FRAGMENT  (WORDS_PER_FRAGMENT*4)


struct frame
{
  uint32_t mid;
  uint32_t fragmentdata[WORDS_PER_FRAGMENT];
  unsigned short metad;
  unsigned short crc16;
};


#define size 32


void
test (void)
{
  struct frame dummy_frame;

  uint8_t sendBuf[32];
  dummy_frame.mid = 0;

  while (1)
    {
      memcpy (sendBuf, &dummy_frame, 32);

      uint16_t crc = crc16 (sendBuf, size - 2);
      sendBuf[size - 2] = (crc >> 8) & 0xff;
      sendBuf[size - 1] = crc & 0xff;

      nrf_send_frame (sendBuf);

      dummy_frame.mid++;
      if (dummy_frame.mid % 8 == 0)
	{
	  delay_ms (2);
	}
      if (dummy_frame.mid % 11 == 0)
	{
	  delay_ms (100);
	}


    }
}

int
main (void)
{

  char outBuf[32];
  struct NRF_CFG config;


  portsetup ();


  /* Configure PB frequency and wait states */
  SYSTEMConfigPerformance (SystemClock ());
//  UART1Init (SystemClock ());
  UART2Init (SystemClock ());

  INTConfigureSystem (INT_SYSTEM_CONFIG_MULT_VECTOR);
  INTEnableInterrupts ();

  UART2PutStr
    (".............................................................................hallo\r\n");
  UART2PutStr ("UART2 Welt\r\n");

//  init_printf ();
  // try to reset nrf chip

  UART2PutStr ("nrf_init(),");

  SPI2_init ();

  nrf_init ();

  UART2PutStr ("done\n\r");

  UART2PutStr ("nrf_config_set(),");
  config.nrmacs = 1;
  config.maclen[0] = 32;
  config.channel = 81;
  ultoa (outBuf, config.channel, 10);
  UART2PutStr ("channel: ,");
  UART2PutStr (outBuf);
  memcpy (config.mac0, "\x01\x02\x03\x02\x01", 5);
  memcpy (config.txmac, "\x1\x2\x3\x2\x1", 5);
  nrf_config_set (&config);

  UART2PutStr ("done\n\r");




  test ();
  while (1);
}
