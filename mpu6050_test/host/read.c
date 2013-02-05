#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <strings.h>

#include <math.h>
						/* baudrate settings are defined in <asm/termbits.h>, which is
						   included by <termios.h> */
#define BAUDRATE B921600
									/* change this definition for the correct port */
#define MODEMDEVICE "/dev/ttyUSB0"
#define _POSIX_SOURCE 1		/* POSIX compliant source */

#define FALSE 0
#define TRUE 1

volatile int STOP = FALSE;

double
pi ()
{
  return (3.1415);
}

int
main (void)
{
  int fd, c, res;
  struct termios oldtio, newtio;
  char buf[255];
  int init_cnt = 0;
  int init = 0;
  int gx_avg = 0;
  int gx_avg_test = 0;
  int gx_avg_test_cnt = 0;
  double vx = 0, vy = 0, vz = 0;
  double x = 0, y = 0, z = 0;

  double agrav_x = 0, agrav_y = 0, agrav_z = 0;
  double acorr_x = 0, acorr_y = 0, acorr_z = 0;

  unsigned int t0a = 0, t1a = 0;
  double gamma_x;		// winkel

  /* 
     Open modem device for reading and writing and not as controlling tty
     because we don't want to get killed if linenoise sends CTRL-C.
   */
  fd = open (MODEMDEVICE, O_RDWR | O_NOCTTY);
  if (fd < 0)
    {
      perror (MODEMDEVICE);
      return (1);
    }

  tcgetattr (fd, &oldtio);	/* save current serial port settings */
  bzero (&newtio, sizeof (newtio));	/* clear struct for new port settings */

  /* 
     BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
     CRTSCTS : output hardware flow control (only used if the cable has
     all necessary lines. See sect. 7 of Serial-HOWTO)
     CS8     : 8n1 (8bit,no parity,1 stopbit)
     CLOCAL  : local connection, no modem contol
     CREAD   : enable receiving characters
   */
  newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

  /*
     IGNPAR  : ignore bytes with parity errors
     ICRNL   : map CR to NL (otherwise a CR input on the other computer
     will not terminate input)
     otherwise make device raw (no other input processing)
   */
  newtio.c_iflag = IGNPAR | ICRNL;

  /*
     Raw output.
   */
  newtio.c_oflag = 0;

  /*
     ICANON  : enable canonical input
     disable all echo functionality, and don't send signals to calling program
   */
  newtio.c_lflag = ICANON;

  /* 
     initialize all control characters 
     default values can be found in /usr/include/termios.h, and are given
     in the comments, but we don't need them here
   */
  newtio.c_cc[VINTR] = 0;	/* Ctrl-c */
  newtio.c_cc[VQUIT] = 0;	/* Ctrl-\ */
  newtio.c_cc[VERASE] = 0;	/* del */
  newtio.c_cc[VKILL] = 0;	/* @ */
  newtio.c_cc[VEOF] = 4;	/* Ctrl-d */
  newtio.c_cc[VTIME] = 0;	/* inter-character timer unused */
  newtio.c_cc[VMIN] = 1;	/* blocking read until 1 character arrives */
  newtio.c_cc[VSWTC] = 0;	/* '\0' */
  newtio.c_cc[VSTART] = 0;	/* Ctrl-q */
  newtio.c_cc[VSTOP] = 0;	/* Ctrl-s */
  newtio.c_cc[VSUSP] = 0;	/* Ctrl-z */
  newtio.c_cc[VEOL] = 0;	/* '\0' */
  newtio.c_cc[VREPRINT] = 0;	/* Ctrl-r */
  newtio.c_cc[VDISCARD] = 0;	/* Ctrl-u */
  newtio.c_cc[VWERASE] = 0;	/* Ctrl-w */
  newtio.c_cc[VLNEXT] = 0;	/* Ctrl-v */
  newtio.c_cc[VEOL2] = 0;	/* '\0' */

  /* 
     now clean the modem line and activate the settings for the port
   */
  tcflush (fd, TCIFLUSH);
  tcsetattr (fd, TCSANOW, &newtio);

  /*
     terminal settings done, now handle input
     In this example, inputting a 'z' at the beginning of a line will 
     exit the program.
   */
  while (STOP == FALSE)
    {
      int i;			/* loop until we have a terminating condition */
      int ax, ay, az;
      int gx, gy, gz;
      unsigned int t0, t1;

      int t;
      res = read (fd, buf, 255);
      buf[res] = 0;		/* set end of string, so we can printf */
//      printf (":%s:%d\n", buf, res);


      i =
	sscanf (buf, "val %u %u %u %u %u %u %u %u %u", &t0, &t1, &ax, &ay, &az,
		&gx, &gy, &gz, &t);


      if (i == 9)		// gültige Zeile 
	{

	  printf
	    ("t0: %u t1: %u   ax: %5d  ay: %5d az: %5d gx: %5d  gy: %5d gz: %5d t: %5d t: %5.3f\n",
	     t0, t1, ax, ay, az, gx, gy, gz,t,  (t / 340. + 36.53));
	}
    }
  /* restore the old port settings */
  tcsetattr (fd, TCSANOW, &oldtio);
}
