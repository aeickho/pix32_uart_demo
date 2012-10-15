#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "base128.h"
#include "crc.h"
#include <termios.h>
#include <sys/fcntl.h>
#include <linits.h>

#define BUFSIZE 1024

#define UART1 "/dev/serial/by-id/usb-Silicon_Labs_CP2102-Alex_0101-if00-port0"


#define	STEP_INIT 		0
#define STEP_WAIT		1
#define STEP_READ		2
#define STEP_RX_NEWFRAME	3
#define STEP_END		-1

#define WORDS_PER_FRAGMENT   6
#define BYTES_PER_FRAGMENT  (WORDS_PER_FRAGMENT*4)

#define FRAMEBUFSIZE	20	

//#if UINT_MAX >= 0xFFFFFFFF
typedef unsigned int  tfec3_u32; /* let's prefer int if wide enough */
//#else
//typedef unsigned long tfec3_u32; /* long is at least 32 bits wide */
//#endif



struct frame {
  tfec3_u32 mid;
  tfec3_u32 fragmentdata[WORDS_PER_FRAGMENT];
  unsigned short metad;
  unsigned short crc16;
  };
           

struct sframe {
  unsigned int	timeout;
  unsigned int  seqnr;
  struct frame  frame;
  };
 
struct msgindex {
  unsigned int msg;
  unsigned int n;
  };
                                
struct sframe frameBuffer[FRAMEBUFSIZE];


static void
print_frame (const struct frame *pf)
{
  int j;
  const char *p;
  p = (const char *) pf->fragmentdata;
  printf ("MID=%08X, [", pf->mid);
  for (j = 0; j < BYTES_PER_FRAGMENT; ++j)
    {
      if (32 <= p[j] && p[j] < 127)
	printf ("%c", p[j]);
      else
	printf (".");
    }
  printf ("], METAD=%04X, CRC16=%04X ", pf->metad, pf->crc16);
}



int
main (int argc, char **argv)
{
  int tty;
  int i;
  unsigned char buf[BUFSIZE];
  unsigned char outBuf[BUFSIZE];
  unsigned short c_crc16 = 12345, r_crc16;
  int step;

  unsigned int seq_nr;
  struct termios termOptions;

  for (i=0;i<FRAMEBUFSIZE;i++)
    {
    frameBuffer[i].timeout=0;
    frameBuffer[i].seqnr=0;
    }
    
 
  tty = open (UART1, O_RDWR | O_NOCTTY);
  if (tty < 0)
    {
      perror ("opening TTY");
      exit (2);
    }

  tcgetattr (tty, &termOptions);

  cfsetispeed (&termOptions, B921600);
  cfsetospeed (&termOptions, B921600);
  tcsetattr (tty, TCSANOW, &termOptions);

  step = STEP_INIT;
  while (step > STEP_END)
    {
      switch (step)
	{
	case STEP_INIT:
	  step = STEP_WAIT;
	  break;
	case STEP_WAIT:
	  read (tty, buf, 1);
	  if (buf[0] == 0x01)
	    step = STEP_READ;
	  break;
	case STEP_READ:

	  read (tty, buf, 8);
	  from_base128n (buf, outBuf, 1);
	  seq_nr = *(unsigned int *) outBuf;
	  printf ("%08d ", seq_nr);

	  read (tty, buf, 40);
	  from_base128n (buf, outBuf, 5);

	  r_crc16 = outBuf[31] << 8 | outBuf[30];
	  c_crc16 = crc16 (outBuf, 30);

/*	  if (r_crc16 != c_crc16)
	    {
	      printf ("checksum error %x %x", r_crc16, c_crc16);
	      step = STEP_WAIT;
	      break;
	    }
*/
/*	  for (i = 0; i < 32; i++)
	    printf ("%02x(%c) ", outBuf[i], outBuf[i] < 32 ? '.' : outBuf[i] > 127 ? '.' : outBuf[i]);
	  printf ("\n");
*/
	  print_frame((const struct frame *) outBuf);

	  for (i = 0; i < 32; i++)
	    printf ("%02x ", outBuf[i]);
	   printf ("%04x %s\n",  c_crc16,r_crc16 == c_crc16?"ok":"nok"); 
	   
	  if (r_crc16 == c_crc16)
            {
            // Add Frame to FameBuffer
            int seq_nr_id;
            int	ii;
            int seq_nr_diff_min;
            struct msgindex msg_index[FRAMEBUFSIZE];   
            
            for(i = 0; i < FRAMEBUFSIZE; i++)
              {
              msg_index[i].id = 0;    
              msg_index[i].n = 0;
              }
              
            for(i = 0; i < FRAMEBUFSIZE; i++)
              if ( seq_nr - frameBuffer[i].seqnr > seq_nr_diff_min)
                {
                seq_nr_diff_min =  seq_nr - frameBuffer[i].seqnr;
                seq_nr_id = i;
                }                        
            memcpy(&frameBuffer[seq_nr_id].frame, outBuf, 32);
            frameBuffer[seq_nr_id].seqnr = seq_nr;
            }

            for(i = 0; i < FRAMEBUFSIZE; i++)
              {
              for(ii=0;ii <	 FRAMEBUFSIZE; ii++)
                {
                if (msg_index[ii].id == frameBuffer[i].frame.mid)
                  {              
                  msg_index[i].n =
              }
                        
            
              
            }    
	  step = STEP_WAIT;
	  break;

	}
    }
  return (0);
}
