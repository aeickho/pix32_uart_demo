#include <p32xxxx.h>
#include <plib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myspi.h"
#include "Pinguino.h"

void wl_module_init (void);

void
wl_module_init (void)
{
  SPI2_init ();
}

void
sspInit (unsigned char portNum, unsigned int dummy1, unsigned int dummy2)
{
  volatile int x;  
  x= portNum + dummy1 + dummy2;
  wl_module_init ();
}

void
sspSend (unsigned char portNum, const unsigned char *buf, unsigned int length)
{
  portNum--;
  SPI2_transmit_sync (buf, length);
}

void
sspReceive (unsigned char portNum, unsigned char *buf, unsigned int length)
{
  portNum--;
//  SPI2_transfer_sync (buf, buf, length);
   SPI2_read (buf, buf[0], length);
   
}

void
sspSendReceive (unsigned char portNum, unsigned char *buf,
		unsigned int length)
{
  portNum--;
  SPI2_transfer_sync (buf, buf, length);
}

void
sspSendReceive0 (unsigned char portNum, unsigned char *inBuf,
		unsigned int length)
{
  portNum--;
  SPI2_read (inBuf, inBuf[0], length);
}


void
delay_1ms (void)
{
#define DELAY 156
  T1CON = 0x8030;
  PR1 = 0xffff;
  TMR1 = 0;
  while (TMR1 < DELAY);


  return;
}

void
delay_7us (void)
{
#define DELAYU 1
  T1CON = 0x8030;
  PR1 = 0xffff;
  TMR1 = 0;
  while (TMR1 < DELAYU);


  return;
}


void
_delay_ms (unsigned int length)
{
  int i;
  for (i = 0; i < length; i++)
    delay_1ms ();
}

void
delay_ms (unsigned int length)
{
  int i;
  for (i = 0; i < length; i++)
    delay_1ms ();
}


