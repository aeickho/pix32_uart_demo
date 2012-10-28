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

#define BUFSIZE 1024

#define UART1 "/dev/serial/by-id/usb-Silicon_Labs_CP2102-Alex_0101-if00-port0"


#define	STEP_INIT 		0
#define STEP_WAIT		1
#define STEP_READ		2
#define STEP_RX_NEWFRAME	3
#define STEP_END		-1

#define WORDS_PER_FRAGMENT   6
#define BYTES_PER_FRAGMENT  (WORDS_PER_FRAGMENT*4)

#define FRAMEBUFSIZE		20
#define MAX_MSG_FRAME_NR	10


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

struct sframe frameBuffer[FRAMEBUFSIZE];	// 800 
struct mid_processeds mid_processed[FRAMEBUFSIZE];
int mid_processed_wp;

int
main (int argc, char **argv)
{
  int tty;
  int i;
  unsigned char buf[BUFSIZE];
  unsigned char outBuf[BUFSIZE];
          unsigned short c_crc16, r_crc16;

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
	  from_base128n (buf, outBuf, 1);
	  seq_nr = *(unsigned int *) outBuf;
	  read (tty, buf, 40);
	  from_base128n (buf, outBuf, 5);

	  r_crc16 = outBuf[31] << 8 | outBuf[30];
	  c_crc16 = crc16 (outBuf, 30);

	  if (r_crc16 == c_crc16)
	    {

	      int flag_ng = 0;
	      struct frame new_frame;

	      memcpy (&new_frame, outBuf, 32);

	      for (i = 0; i < FRAMEBUFSIZE; i++)
		{
		  if (mid_processed[i].mid == new_frame.mid)
		    {
		      if (mid_processed[i].cnt == -1)
			{
			  flag_ng = -1;
			  break;
			}
		      mid_processed[i].cnt++;
		      flag_ng = -2;
		      if (mid_processed[i].cnt > 6)
			{
			  flag_ng = -3;
			  mid_processed[i].cnt = -1;
			}
		    }
		}

	      if (flag_ng == -1)
		{
		  step = STEP_WAIT;
		  break;
		}

	      if (flag_ng == 0)
		{
		  mid_processed[mid_processed_wp].mid = new_frame.mid;
		  mid_processed[mid_processed_wp].cnt = 1;
		  mid_processed_wp++;
		  mid_processed_wp %= FRAMEBUFSIZE;
		}

	      int seq_nr_diff_min = 0;
	      int seq_nr_id;

	      for (i = 0; i < FRAMEBUFSIZE; i++)
		{
		  if (seq_nr - frameBuffer[i].seqnr > seq_nr_diff_min)
		    {
		      seq_nr_diff_min = seq_nr - frameBuffer[i].seqnr;
		      seq_nr_id = i;
		    }
		}

	      memcpy (&frameBuffer[seq_nr_id].frame, outBuf, 32);
	      frameBuffer[seq_nr_id].seqnr = seq_nr;

	      if (flag_ng == -3)
		{
		  unsigned char tx_valid[FRAME_BUFF_SIZE];
		  tfec3_u32 *fragdatas[FRAME_BUFF_SIZE];
		  struct frame spare_frame[3];

		  int tx_valid_max = 0;
		  char nr_data_frames;

		  memset (tx_valid, 0, sizeof (tx_valid));
		  memset (fragdatas, 0, nr_data_frames + 3);

		  for (i = 0; i < FRAMEBUFSIZE; i++)
		    {
		      if (frameBuffer[i].frame.mid == new_frame.mid)
			{
			  tx_valid[FRAGMENT_INDEX (&frameBuffer[i].frame)] =
			    1;
			  fragdatas[FRAGMENT_INDEX (&frameBuffer[i].frame)]
			    = frameBuffer[i].frame.fragmentdata;
			  nr_data_frames =
			    (NUM_DATA_FRAGS (&frameBuffer[i].frame));
			  if ((FRAGMENT_INDEX (&frameBuffer[i].frame)) >
			      tx_valid_max)
			    tx_valid_max =
			      FRAGMENT_INDEX (&frameBuffer[i].frame);
			}
		    }

		  for (int ii = 0, i = 0; i < nr_data_frames + 3; i++)
		    {
		      if (tx_valid[i] == 0)
			{
			  fragdatas[i] = spare_frame[ii++].fragmentdata;
			}
		    }

		  tfec3_decode (WORDS_PER_FRAGMENT, nr_data_frames,
				tx_valid_max - nr_data_frames + 1, tx_valid,
				fragdatas);

		  for (i = 0; i < nr_data_frames; i++)
		    {
		      int ii;
		      char **p;
		      p = (char **) fragdatas;
		      printf ("[");
		      for (ii = 0; ii < 24; ii++)
			{
			  if (32 <= p[i][ii] && p[i][ii] < 127)
			    printf ("%c", p[i][ii]);
			  else
			    printf (".");
			}
		      printf ("]\n");
		    }
		}
	    }
	  step = STEP_WAIT;
	  break;
	}
    }
  return (0);
}
