#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "base128.h"

#define BUFSIZE 1024

#define UART1 "/dev/serial/by-id/usb-Silicon_Labs_CP2102-Alex_0104-if00-port0"


#define	STEP_INIT 	0
#define STEP_WAIT	1
#define STEP_READ	2
#define STEP_END	-1

int
main (int argc, char **argv)
{
  int tty;
  FILE *in;
  int c, i;
  char val;
  unsigned char buf[BUFSIZE]={0xff,0xfe,0xfd,0xfc,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
  
  unsigned char outBuf[BUFSIZE];
  unsigned int sector = 0;
  unsigned short s_crc16 = 12345, r_crc16;
  int step;
  time_t start_time;
  int buf_pos = 0;
  

  tty = open (UART1, O_RDWR | O_NOCTTY );
  if (tty < 0)
    {
      perror ("opening TTY");
      exit (2);
    }

  step = STEP_INIT;
  while (step > STEP_END)
    {
      switch (step)
	{
	case STEP_INIT:
	  step = STEP_WAIT;
	  break;
	case STEP_WAIT:
	  c = read (tty, buf, 1);
	  if (buf[0]==0x01)
	  step = STEP_READ;
	  break;
	case STEP_READ:
	  c = read (tty, buf, 40);
          from_base128n(buf,outBuf,5);
          for (i=0; i<32;i++)
            printf ("%02x ",outBuf[i]);
           printf ("\n");  
          step = STEP_WAIT;
          break;
        }
      }  
}          
        