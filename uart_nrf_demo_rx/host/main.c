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
#include <limits.h>


#include "aeickho-tiny-fecc/tfec3.h"
#include "rxmsg.h"

#define BUFSIZE 1024

#define UART1 "/dev/serial/by-id/usb-Silicon_Labs_CP2102-Alex_0105-if00-port0"

#define	STEP_INIT 		0
#define STEP_WAIT		1
#define STEP_READ		2
#define STEP_RX_NEWFRAME	3
#define STEP_END		-1

#define FLAG_NEW_MESSAGE	0
#define FLAG_MID_PROCESSED	1
#define FLAG_MID_COMPLEATE	2

#define FRAMEBUFSIZE		15

#define WORDS_PER_FRAGMENT   6
#define BYTES_PER_FRAGMENT  (WORDS_PER_FRAGMENT*4)
#define MAX_DATA_FRAGMENTS  15
#define MAX_RECOV_FRAGMENTS  3
#define MAX_MESSAGE_SIZE    (BYTES_PER_FRAGMENT*MAX_DATA_FRAGMENTS)
#define FRAME_BUFF_SIZE     (MAX_DATA_FRAGMENTS+MAX_RECOV_FRAGMENTS)

#define NUM_DATA_FRAGS(pf)  ((pf)->metad >> 12)
#define FRAGMENT_INDEX(pf)  ((pf)->metad &  63)

typedef unsigned int tfec3_u32;	/* let's prefer int if wide enough */

struct frame
{
  tfec3_u32 mid;
  tfec3_u32 fragmentdata[WORDS_PER_FRAGMENT];
  unsigned short metad;
  unsigned short crc16;
};

struct sframe
{
  unsigned int timeout;
  unsigned int seqnr;
  struct frame frame;
};

struct mid_processeds
{
  tfec3_u32 mid;
  int cnt;
};

#define MID_PROCESSED_SIZE 5
struct sframe frameBuffer[FRAMEBUFSIZE];	// 800 
struct mid_processeds mid_processed[MID_PROCESSED_SIZE];
int mid_processed_wp;

int
main (int argc, char **argv)
{
  int tty;
  unsigned char buf[BUFSIZE];
  unsigned char inData[BUFSIZE];
  unsigned short c_crc16, r_crc16;

  unsigned char outData[BYTES_PER_FRAGMENT * MAX_DATA_FRAGMENTS];


  int step;
  FILE *rawfile;

  unsigned int seq_nr;
  struct termios termOptions;

  tty = open (UART1, O_RDWR | O_NOCTTY);
  if (tty < 0)
    {
      perror ("opening TTY....");
      exit (2);
    }

  rawfile = fopen ("raw.txt", "w");
  if (rawfile == NULL)
    {
      perror ("raw.txt");
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
	  from_base128n (buf, inData, 1);
	  seq_nr = *(unsigned int *) inData;
	  read (tty, buf, 40);
	  from_base128n (buf, inData, 5);

	  r_crc16 = inData[31] << 8 | inData[30];
	  c_crc16 = crc16 (inData, 30);

	  for (int i = 0; i < 32; i++)
	    printf ("%02x ", inData[i]);


	  if (r_crc16 != c_crc16)
	    printf ("falsche crc\n");
	  if (r_crc16 == c_crc16)
	    {

	      int i, ii;
	      int anz;
	      int nr_data_frames;
	      anz = rxmsg_process_frame (inData, outData);
	      if (anz != 0)
		{
		  nr_data_frames = anz / BYTES_PER_FRAGMENT;
		  printf ("\n");

		  for (i = 0; i < anz; i++)
		    {
		      printf ("[\n");
		      for (ii = 0; ii < 24; ii++)
			{
			  unsigned char c;
			  c = outData[i + ii];
			  if (32 <= c && c < 127)
			    printf ("%c", c);
			  else
			    printf (".");
			}
		      printf ("]\n");
		    }
		  printf ("-> %d %d\n",
			  BYTES_PER_FRAGMENT * nr_data_frames,
			  nr_data_frames);
		}
	    }
	}
      step = STEP_WAIT;
      break;
    }

return (0);
}
