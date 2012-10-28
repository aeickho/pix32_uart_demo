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



//#if UINT_MAX >= 0xFFFFFFFF
typedef unsigned int tfec3_u32;	/* let's prefer int if wide enough */
//#else
//typedef unsigned long tfec3_u32; /* long is at least 32 bits wide */
//#endif



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

struct msgindex
{
  unsigned int msg;
  unsigned int n;
  unsigned char used;
};



struct sframe frameBuffer[FRAMEBUFSIZE];	// 800 

struct sframe frameBufferNG[FRAMEBUFSIZE];

struct mid_processeds
{
  tfec3_u32 mid;
  int cnt;
};

struct mid_processeds mid_processed[FRAMEBUFSIZE];
int mid_processed_wp;



#define NUM_DATA_FRAGS(pf)  ((pf)->metad >> 12)
#define FRAGMENT_INDEX(pf)  ((pf)->metad &  63)



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
  unsigned int delete_msg_id[FRAMEBUFSIZE];	// 80
  int delete_msg_id_idx = 0;
  FILE *rawfile;

  unsigned int seq_nr;
  struct termios termOptions;

  for (i = 0; i < FRAMEBUFSIZE; i++)
    {
      frameBuffer[i].timeout = 0;
      frameBuffer[i].seqnr = 0;
      delete_msg_id[i] = 0;
    }


  for (i = 0; i < FRAMEBUFSIZE; i++)
    {
      frameBufferNG[i].timeout = 0;
      frameBufferNG[i].seqnr = 0;
      mid_processed[i].cnt = 0;
    }

  mid_processed_wp = 0;



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

	  printf ("%d :", seq_nr);


	  for (i = 0; i < 32; i++)
	    printf ("%02x ", outBuf[i]);

	  printf ("%04x %s\n", c_crc16, r_crc16 == c_crc16 ? "ok" : "nok");


	  if (r_crc16 == c_crc16)
	    {

	      int flag_ng = 0;
	      struct frame new_frame_ng;

	      memcpy (&new_frame_ng, outBuf, 32);

	      printf ("new _ frame _mid :%x \n", new_frame_ng.mid);
	      // check for: if mid processed
	      for (i = 0; i < FRAMEBUFSIZE; i++)
		{
		  if (mid_processed[i].mid == new_frame_ng.mid)
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
		  goto a;
		  step = STEP_WAIT;
		  break;
		}

	      if (flag_ng == 0)
		{
		  mid_processed[mid_processed_wp].mid = new_frame_ng.mid;
		  mid_processed[mid_processed_wp].cnt = 1;
		  mid_processed_wp++;
		  mid_processed_wp %= FRAMEBUFSIZE;
		}

	      int seq_nr_diff_min = 0;
	      int seq_nr_id;

	      for (i = 0; i < FRAMEBUFSIZE; i++)
		{
		  if (seq_nr - frameBufferNG[i].seqnr > seq_nr_diff_min)
		    {
		      seq_nr_diff_min = seq_nr - frameBufferNG[i].seqnr;
		      seq_nr_id = i;
		    }
		}

	      memcpy (&frameBufferNG[seq_nr_id].frame, outBuf, 32);
	      frameBufferNG[seq_nr_id].seqnr = seq_nr;



	      if (flag_ng == -3)
		{
		  unsigned char tx_valid[FRAME_BUFF_SIZE];
		  tfec3_u32 *fragdatas[FRAME_BUFF_SIZE];
		  int tx_valid_max = 0;
		  char nr_data_frames;
		  struct frame spare_frame[3];
		  memset (tx_valid, 0, sizeof (tx_valid));


		  for (i = 0; i < nr_data_frames + 3; i++)
		    fragdatas[i] = NULL;

		  for (i = 0; i < FRAMEBUFSIZE; i++)
		    {
		      if (frameBufferNG[i].frame.mid == new_frame_ng.mid)
			{
			  tx_valid[FRAGMENT_INDEX (&frameBufferNG[i].frame)] =
			    1;
			  fragdatas[FRAGMENT_INDEX (&frameBufferNG[i].frame)]
			    = frameBufferNG[i].frame.fragmentdata;
			  if ((FRAGMENT_INDEX (&frameBufferNG[i].frame)) >
			      tx_valid_max)
			    tx_valid_max =
			      FRAGMENT_INDEX (&frameBufferNG[i].frame);
			  nr_data_frames =
			    (NUM_DATA_FRAGS (&frameBufferNG[i].frame));
			}
		    }




		  int ii = 0;
		  for (i = 0; i < nr_data_frames + 3; i++)
		    {
		      if (tx_valid[i] == 0)
			{
			  fragdatas[i] = spare_frame[ii++].fragmentdata;
			}
		    }

		  printf ("========== \n");

		  for (i = 0; i < nr_data_frames + 3; i++)
		    {
		      printf ("%d %p \n", i, fragdatas[i]);
		    }

		  printf ("========== \n");



		  int s, k;
		  s = nr_data_frames;

		  k = tx_valid_max - nr_data_frames + 1;	// ???????????????????
		  printf ("\n nr_data_frames %d tx_valid_max %d k %d \n",
			  nr_data_frames, tx_valid_max, k);

		  if (nr_data_frames - 1 == tx_valid_max)
		    {
		      printf ("alles ok ohne tfec3\n");
		    }
		  else
		    {
		      ii =
			tfec3_decode (WORDS_PER_FRAGMENT, s, k, tx_valid,
				      fragdatas) ? s : 0;

		    }


		  for (i = 0; i < nr_data_frames; i++)
		    {
		      int ii;
		      char ** p;
		      p= (char*) fragdatas;
		      for (ii = 0; ii < 24; ii++)
			{
			  if (32 <= p[i][ii]
			      && p[i][ii] < 127)
                	    printf ("%c", p[i][ii]);
			  else
			    printf (".");
			}

		      printf ("\n");
		    }

		  printf ("========== \n");
		  for (i = 0; i < FRAMEBUFSIZE; i++)
		    {
		      for (int ii = 0; ii < FRAMEBUFSIZE; ii++)
			{
			  if (frameBufferNG[ii].frame.mid ==
			      new_frame_ng.mid
			      && i == (frameBufferNG[ii].frame.metad & 0x1f))
			    {
			      printf ("%d %d %x %d %d\n", i, ii,
				      frameBufferNG[ii].frame.mid,
				      frameBufferNG[ii].seqnr,
				      frameBufferNG[ii].frame.metad & 0x1f);
			    }
			}
		    }
		}

	    }
	a:
	  if (r_crc16 == c_crc16)
	    {
	      int seq_nr_id;
	      int ii;
	      int seq_nr_diff_min = 0;
	      struct msgindex msg_index[FRAMEBUFSIZE];	// 180
	      struct frame msg_buff[FRAMEBUFSIZE][MAX_MSG_FRAME_NR];	// 7200 
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
	      for (i = 0; i < FRAMEBUFSIZE; i++)
		{
		  msg_index[i].msg = 0;
		  msg_index[i].n = 0;
		  for (ii = 0; ii < MAX_MSG_FRAME_NR; ii++)
		    msg_buff[i][ii].metad = 0;
		}


	      for (i = 0; i < FRAMEBUFSIZE; i++)
		{
		  int ii;
		  int msg_id;
		  int fnum;
		  fnum = frameBuffer[i].frame.metad & 0x1f;
		  msg_id = frameBuffer[i].frame.mid;
		  for (ii = 0; ii < FRAMEBUFSIZE; ii++)
		    {
		      if ((msg_index[ii].msg == msg_id) && msg_id != 0)
			{
			  msg_index[ii].n++;
			  memcpy (&msg_buff[ii][fnum],
				  &frameBuffer[i].frame, 32);
			  break;
			}
		      else
			{
			  if (msg_index[ii].n == 0 && msg_id != 0)
			    {
			      msg_index[ii].msg = msg_id;
			      msg_index[ii].n++;
			      memcpy (&msg_buff[ii][fnum],
				      &frameBuffer[i].frame, 32);
			      break;
			    }
			}
		    }
		}

	      for (i = 0; i < FRAMEBUFSIZE; i++)
		{
		  int f = 0;
		  for (ii = 0; ii < FRAMEBUFSIZE; ii++)
		    if (delete_msg_id[ii] == msg_index[i].msg)
		      f = 1;
		  if (msg_index[i].n > 6 && msg_index[i].msg && f == 0)
		    {
		      printf ("xxxxxxx\n");
		      for (ii = 0; ii < 9; ii++)
			{
			  printf ("%01d: ", ii);
			  if (msg_buff[i][ii].mid == msg_index[i].msg)
			    print_frame ((const struct frame *)
					 &msg_buff[i][ii]);
			  printf ("\n");
			}
		      delete_msg_id[delete_msg_id_idx++] = msg_index[i].msg;
		      delete_msg_id_idx %= FRAMEBUFSIZE;
		    }
		}
	    }

	  step = STEP_WAIT;
	  break;
	}
    }
  return (0);
}
