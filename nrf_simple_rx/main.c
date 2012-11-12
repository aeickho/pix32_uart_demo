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
#include "tools/printf.h"
#include "nrf_x.h"
#include "aeickho-tiny-fecc/tfec3.h"
#include "rxmsg.h"


#define WORDS_PER_FRAGMENT   6
#define BYTES_PER_FRAGMENT  (WORDS_PER_FRAGMENT*4)

/*
struct frame
{
  uint32_t mid;
  uint32_t fragmentdata[WORDS_PER_FRAGMENT];
  unsigned short metad;
  unsigned short crc16;
};
*/

#define size 32

#define FLAG_NEW_MESSAGE        0
#define FLAG_MID_PROCESSED      1
#define FLAG_MID_COMPLEATE      2

#define FRAMEBUFSIZE            15+3


#define MAX_DATA_FRAGMENTS  15
#define MAX_RECOV_FRAGMENTS  3

#define FRAME_BUFF_SIZE     (MAX_DATA_FRAGMENTS+MAX_RECOV_FRAGMENTS)

#define NUM_DATA_FRAGS(pf)  ((pf)->metad >> 12)
#define FRAGMENT_INDEX(pf)  ((pf)->metad &  63)



struct frame
{
  uint32_t mid;
  uint32_t fragmentdata[WORDS_PER_FRAGMENT];
  uint16_t metad;
  uint16_t crc16;
};

struct sframe
{
  uint32_t timeout;
  uint32_t seqnr;
  struct frame frame;
};

struct mid_processeds
{
  uint32_t mid;
  uint32_t cnt;
};



#define MID_PROCESSED_SIZE 5

struct sframe frameBuffer[FRAMEBUFSIZE];	// 800 
struct mid_processeds mid_processed[MID_PROCESSED_SIZE];
int mid_processed_wp;








int
main (void)
{

  char outBuf[32];
  struct NRF_CFG config;
  uint8_t inData[32];
  unsigned char outData[24 * (15+3)];
  unsigned int seq_nr;

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

  init_printf ();
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


  UART2PutStr ("nrf_receive_start");
  nrf_receive_start ();
  UART2PutStr ("done\n\r");

  while (1)
    {
      int ret;

      ret = nrf_receive_poll (inData);
      if (ret == 32)
	{
	  int i;
	  unsigned short c_crc16, r_crc16;
	  struct frame *new_frame;

	  r_crc16 = inData[31] << 8 | inData[30];
	  c_crc16 = crc16 (inData, 30);

	  if (r_crc16 == c_crc16)
	    {
	      int i, ii;
	      int anz;
	      int nr_data_frames;
	      int t[10];
	      
	      
	      t[0]=ReadCoreTimer ();
	      rxmsg_process_frame (inData );
	      t[1]=ReadCoreTimer ();
	      anz=rxmsg_get_frame(  outData);
	      t[2]=ReadCoreTimer ();
	      
	      
    	      if (anz != 0)
		{
		  nr_data_frames = anz / BYTES_PER_FRAGMENT;

		  for (i = 0; i < anz; i = i + 24)
		    {
		      tfp_printf ("[");
		      for (ii = 0; ii < 24; ii++)
			{
			  unsigned char c;
			  c = outData[i + ii];
			  if (32 <= c && c < 127)
			    tfp_printf ("%c", c);
			  else
			    tfp_printf (".");
			}
		      tfp_printf ("]\n\r");
		    }
		  tfp_printf ("-> %d %d\r\n",
			  BYTES_PER_FRAGMENT * nr_data_frames,
			  nr_data_frames);
	      tfp_printf("rxmsg %d %d us\n\r",((t[1]-t[0])*50)/1000, ((t[2]-t[1])*50)/1000);

		}
	    }
	}
    }
}
