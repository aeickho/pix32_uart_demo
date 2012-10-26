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

#define WORDS_PER_FRAGMENT   6
#define BYTES_PER_FRAGMENT  (WORDS_PER_FRAGMENT*4)


struct frame
{
  uint32_t mid;
  uint32_t fragmentdata[WORDS_PER_FRAGMENT];
  unsigned short metad;
  unsigned short crc16;
};




void
test (void)
{
  struct frame dummy_frame;

  dummy_frame.mid = 0;

  while (1)
    {
      nrf_send_frame ((uint8_t *) &dummy_frame);
      dummy_frame.mid++;
    }
}

int
main (void)
{
  struct NRF_CFG config;

  portsetup ();

  SYSTEMConfigPerformance (SystemClock ());

  UART2Init (SystemClock ());

  INTConfigureSystem (INT_SYSTEM_CONFIG_MULT_VECTOR);
  INTEnableInterrupts ();

  UART2PutStr ("............................hallo\r\n");
  UART2PutStr ("UART2 Welt\r\n");

//  init_printf ();

  UART2PutStr ("nrf_init(),");
  nrf_init ();
  UART2PutStr ("done\n\r");

  UART2PutStr ("nrf_config_set(),");
  config.nrmacs = 1;
  config.maclen[0] = 32;
  config.channel = 81;
  memcpy (config.mac0, "\x01\x02\x03\x02\x01", 5);
  memcpy (config.txmac, "\x1\x2\x3\x2\x1", 5);
  nrf_config_set (&config);
  UART2PutStr ("done\n\r");

  UART2PutStr ("call\n\r");
  test ();
  while (1);
}
