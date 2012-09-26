#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>


#define BUFSIZE 1024

#define UART1 "/dev/serial/by-id/usb-Silicon_Labs_CP2102-Alex_0104-if00-port0"


#define	STEP_INIT 	0
#define STEP_SEND	1
#define STEP_READ	2
#define STEP_READ_STRING_OK	3
#define STEP_END 	-1



int
main (int argc, char **argv)
{
  int tty;
  FILE *in;
  int c, i;
  char val;
  char buf[BUFSIZE];
  unsigned int sector = 0;
  unsigned short crc16=12345;
  int step;
  time_t start_time;
  int buf_pos = 0;

  if (argc < 3)
    {
      fprintf (stderr,
	       "you need to give the tty device as first argument.\n");
      exit (1);
    }

  tty = open (UART1, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (tty < 0)
    {
      perror ("opening TTY");
      exit (2);
    }

  in = fopen (argv[1], "r");
  if (in == NULL)
    {
      perror ("opening infile");
      exit (2);
    }

  sector = atoi (argv[2]);

//  sync
  printf ("waiting for sync");
  write (tty, "\nhallo\n", 7);
  
  step = STEP_INIT;
  while (step > STEP_END)
    {
      switch (step)
	{
	case STEP_INIT:
	  printf ("flush buffer\n");
	  c = read (tty, buf, 1024);
	  step = STEP_SEND;
	  break;
	case STEP_SEND:
	  printf ("STEP_SEND\n");
	  write (tty, "\n", 1);
	  start_time = time (NULL);
	  buf_pos = 0;
	  step = 2;
	  break;
	case STEP_READ:
	printf ("STEP_READ\n");
	  if (time (NULL) > start_time + 2)
	    {
	      step = STEP_SEND;
	      printf ("timeout\n");
	     sleep(1);
	      break;
	    }

	  c = read (tty, &buf[buf_pos], 1);
	  buf[buf_pos + 1] = '\0';
          
	  if (buf[buf_pos] == '\n')
	    {
	      buf[buf_pos + 1] = '\0';
	      step = STEP_READ_STRING_OK;
	      break;
	    }

	  if (c > 0 && buf_pos < (sizeof (buf) - 1))
	    {
	  printf ("--%s--\n",buf);
	      buf_pos++;
	    }

	  break;

	case STEP_READ_STRING_OK:
	printf ("STEP_READ_STRING_OK: ");
	  if (!strncmp (buf, "ready", strlen ("ready")))
	    {
            printf ("YES\n");	      step = STEP_END;
	      break;
	    }
	  else
	    {
	    printf ("NO\n"); 
	      step = STEP_SEND;
	      break;
	    }
	}
    }

  printf ("sendblock\n");
  write (tty, "sendblock\n", strlen ("sendblock\n"));
  printf ("sector %d \n", sector);
  write (tty, &sector, 4);
  printf("x: %x\n",sector);
  printf ("crc %d \n", crc16);
  write (tty, &crc16, 2);
  
//  fread (buf, 1, 512, in);
//  write (tty, buf, 512);


}
