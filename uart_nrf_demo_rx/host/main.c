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
  int i;
  unsigned char buf[BUFSIZE];
  unsigned char outBuf[BUFSIZE];
  unsigned short c_crc16 = 12345, r_crc16;
  int step;

  unsigned int seq_nr;  
  struct termios termOptions;


  tty = open (UART1, O_RDWR | O_NOCTTY );
  if (tty < 0)
    {
      perror ("opening TTY");
      exit (2);
    }

  tcgetattr( tty, &termOptions );
  
  cfsetispeed( &termOptions, B921600 );
  cfsetospeed( &termOptions, B921600 );
  tcsetattr( tty, TCSANOW, &termOptions );

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
	  if (buf[0]==0x01)
	  step = STEP_READ;
	  break;
	case STEP_READ:

	  read (tty, buf, 8);
          from_base128n(buf,outBuf,1);
          seq_nr = * (unsigned  int *) outBuf;
          printf ("%08d ",seq_nr);  

	  read (tty, buf, 40);
          from_base128n(buf,outBuf,5);

          r_crc16=outBuf[30]<<8 | outBuf[31];
          c_crc16=crc16(outBuf,30);
          
          if ( r_crc16 != c_crc16)
            {
            printf("checksum error %x %x", r_crc16, c_crc16);          
            step = STEP_WAIT;
            break;
            }
         for (i=0; i<32;i++)
           printf ("%02x ",outBuf[i]);
                        
                        


           printf ("\n");  
          step = STEP_WAIT;
          break;
        }
      }  
return (0);
}          
        