#include "uart.h"

#include "general_exception_handler.h"

volatile struct UARTFifo UART2Fifo;

#define BUFSIZE 300

static uint8_t pIn[BUFSIZE];
static uint8_t pOut[BUFSIZE];


// Init output fifo
void
UART2FifoInit (void)
{
  UART2Fifo.in_read_pos = 0;
  UART2Fifo.in_write_pos = 0;
  UART2Fifo.out_read_pos = 0;
  UART2Fifo.out_write_pos = 0;
  UART2Fifo.in_nchar = 0;
  UART2Fifo.out_nchar = 0;
  UART2Fifo.bufsize = BUFSIZE;

  UART2Fifo.in = pIn;
  UART2Fifo.out = pOut;
}

void
ToUART2Fifo_in (const uint8_t character)
{
  UART2Fifo.in[UART2Fifo.in_write_pos] = character;
  UART2Fifo.in_write_pos = (UART2Fifo.in_write_pos + 1);
  if (UART2Fifo.in_write_pos == UART2Fifo.bufsize)
    UART2Fifo.in_write_pos = 0;
  UART2Fifo.in_nchar++;
}

void __attribute__ ((nomips16)) ToUART2Fifo_out (const uint8_t character)
{
  unsigned int status = 0;

  UART2Fifo.out[UART2Fifo.out_write_pos] = character;
  UART2Fifo.out_write_pos = (UART2Fifo.out_write_pos + 1);
  if (UART2Fifo.out_write_pos == UART2Fifo.bufsize)
    UART2Fifo.out_write_pos = 0;

  asm volatile ("di    %0":"=r" (status));
  UART2Fifo.out_nchar++;
  asm volatile ("ei    %0":"=r" (status));
}

int __attribute__ ((nomips16)) FromUART2Fifo_in (void)
{
  int in = -1;
  unsigned int status = 0;


  if (UART2Fifo.in_nchar > 0)
    {
      in = UART2Fifo.in[UART2Fifo.in_read_pos];
      UART2Fifo.in_read_pos = (UART2Fifo.in_read_pos + 1);
      if (UART2Fifo.in_read_pos == UART2Fifo.bufsize)
	UART2Fifo.in_read_pos = 0;
      asm volatile ("di    %0":"=r" (status));
      UART2Fifo.in_nchar--;
      asm volatile ("ei    %0":"=r" (status));
    }
  return (in);
}

int
FromUART2Fifo_out ()
{
  int in = -1;

  if (UART2Fifo.out_nchar > 0)
    {
      in = UART2Fifo.out[UART2Fifo.out_read_pos];
      UART2Fifo.out_read_pos = (UART2Fifo.out_read_pos + 1);
      if (UART2Fifo.out_read_pos == UART2Fifo.bufsize)
	UART2Fifo.out_read_pos = 0;
      UART2Fifo.out_nchar--;
    }
  return (in);
}

int
UART2ReadChar ()
{
  return FromUART2Fifo_in ();
}


void
UART2Read (uint8_t * buf, const uint16_t n)
{
  int i;
  int ret;

  for (i = 0; i < n; i++)
    {
      do
	ret = FromUART2Fifo_in ();
      while (ret < 0);

      buf[i] = (uint8_t) ret;
    }
}



int
UART2Fifo_out_get_nchar (void)
{
  return (UART2Fifo.out_nchar);
}

int
UART2Fifo_in_get_nchar (void)
{
  return (UART2Fifo.in_nchar);
}



void
UART2SendTrigger (void)
{
  INTEnable (INT_SOURCE_UART_TX (UART2), INT_ENABLED);
}


void
UART2Send (const uint8_t * buffer, UINT32 size)
{
  while (size)
    {
      while (UART2Fifo.out_nchar > BUFSIZE - 2);
      ToUART2Fifo_out (*buffer);
      buffer++;
      size--;
    }
  UART2SendTrigger ();
}


void
UART2PutHexChar (const int val)
{
  if (val > 9)
    {
      UART2SendChar ((char) ('A' - 10) + val);
    }
  else
    {
      UART2SendChar ((char) '0' + val);
    }

}

void
UART2PutHex (unsigned int val)
{
  register unsigned int i;
  for (i = 0; i < 8; i++)
    {
      UART2PutHexChar ((val & 0xF0000000) >> 28);
      val <<= 4;
    }
}

void
UART2PutCharHex (unsigned char val)
{
  UART2PutHexChar ((val & 0xF0) >> 4);
  UART2PutHexChar ((val & 0x0F));
}


void
UART2SendChar (const uint8_t character)
{
  while (UART2Fifo.out_nchar > BUFSIZE - 2);
  ToUART2Fifo_out (character);
  UART2SendTrigger ();
}

void
UART2PutStr (const char *buffer)
{
  UART2Send ((uint8_t *) buffer, strlen (buffer));
}

void
  __attribute__ ((nomips16, interrupt (ipl2),
		  vector (_UART_2_VECTOR))) uart2_interrupt (void)
{
  uint8_t in;

  if (INTGetFlag (INT_SOURCE_UART_RX (UART2)))
    {

      if (U2STAbits.URXDA)
	{
	  in = U2RXREG;
	  if ((char) in == 'B')
	    {
	      _general_exception_handler ();
	    }

	  ToUART2Fifo_in (in);
	}
      INTClearFlag (INT_SOURCE_UART_RX (UART2));
    }

  if (INTGetFlag (INT_SOURCE_UART_TX (UART2)))
    {
      if (UART2Fifo.out_nchar == 0)
	{
	  INTEnable (INT_SOURCE_UART_TX (UART2), INT_DISABLED);
	}
      else
	{
	  while (UART2Fifo.out_nchar > 0)
	    {
	      if (U2STAbits.UTXBF)
		{
		  break;
		}
	      U2TXREG = (char) FromUART2Fifo_out ();
	    }
	}
      INTClearFlag (INT_SOURCE_UART_TX (UART2));
    }
}

void
UART2Init (uint32_t SystemClock)
{
#define GetPeripheralClock()            (SystemClock/(1 << OSCCONbits.PBDIV))

  UART2FifoInit ();

  RPC9R = 2;
  U2RXR = 6;

  UARTConfigure (UART2, UART_ENABLE_PINS_TX_RX_ONLY | UART_ENABLE_HIGH_SPEED);
  UARTSetFifoMode (UART2,
		   UART_INTERRUPT_ON_TX_BUFFER_EMPTY |
		   UART_INTERRUPT_ON_RX_NOT_EMPTY);
  UARTSetLineControl (UART2,
		      UART_DATA_SIZE_8_BITS | UART_PARITY_NONE |
		      UART_STOP_BITS_1);

  UARTSetDataRate (UART2, GetPeripheralClock (), 921600);
  UARTEnable (UART2, UART_ENABLE_FLAGS (UART_PERIPHERAL | UART_RX | UART_TX));
  INTEnable (INT_SOURCE_UART_RX (UART2), INT_ENABLED);
  INTSetVectorPriority (INT_VECTOR_UART (UART2), INT_PRIORITY_LEVEL_2);
  INTSetVectorSubPriority (INT_VECTOR_UART (UART2), INT_SUB_PRIORITY_LEVEL_0);
}
