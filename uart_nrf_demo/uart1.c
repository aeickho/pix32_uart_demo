#include "uart.h"


volatile struct UARTFifo UART1Fifo;

// Init output fifo
void
UART1FifoInit (void)
{
  UART1Fifo.in_read_pos = 0;
  UART1Fifo.in_write_pos = 0;
  UART1Fifo.out_read_pos = 0;
  UART1Fifo.out_write_pos = 0;
  UART1Fifo.in_nchar = 0;
  UART1Fifo.out_nchar = 0;
  UART1Fifo.bufsize = 300;

  UART1Fifo.in = malloc (UART1Fifo.bufsize);
  UART1Fifo.out = malloc (UART1Fifo.bufsize);
}


// Add one character to output fifo 
void
ToUART1Fifo_in (const uint8_t character)
{
  UART1Fifo.in[UART1Fifo.in_write_pos] = character;
  UART1Fifo.in_write_pos = (UART1Fifo.in_write_pos + 1);
  if (UART1Fifo.in_write_pos == UART1Fifo.bufsize)
    UART1Fifo.in_write_pos = 0;
  UART1Fifo.in_nchar++;
}

void
ToUART1Fifo_out (const uint8_t character)
{
  UART1Fifo.out[UART1Fifo.out_write_pos] = character;
  UART1Fifo.out_write_pos = (UART1Fifo.out_write_pos + 1);
  if (UART1Fifo.out_write_pos == UART1Fifo.bufsize)
    UART1Fifo.out_write_pos = 0;
  UART1Fifo.out_nchar++;
}

int
FromUART1Fifo_in (void)
{
  int in = -1;
  if (UART1Fifo.in_nchar > 0)
    {
      in = UART1Fifo.in[UART1Fifo.in_read_pos];
      UART1Fifo.in_read_pos = (UART1Fifo.in_read_pos + 1);
      if (UART1Fifo.in_read_pos == UART1Fifo.bufsize)
	UART1Fifo.in_read_pos = 0;
      UART1Fifo.in_nchar--;
    }
  return (in);
}



int
FromUART1Fifo_out ()
{
  int in = -1;
  if (UART1Fifo.out_nchar > 0)
    {
      in = UART1Fifo.out[UART1Fifo.out_read_pos];
      UART1Fifo.out_read_pos = (UART1Fifo.out_read_pos + 1);
      if (UART1Fifo.out_read_pos == UART1Fifo.bufsize)
	UART1Fifo.out_read_pos = 0;
      UART1Fifo.out_nchar--;
    }
  return (in);
}

inline int
UART1ReadChar (void)
{
  return FromUART1Fifo_in ();
}

int
UART1Read (uint8_t * buf, const uint16_t n)
{
  int i;
  int ret;

  for (i = 0; i < n; i++)
    {
      do
	ret = FromUART1Fifo_in ();
      while (ret < 0);

      buf[i] = (uint8_t) ret;
    }
}

inline int
UART1Fifo_out_get_nchar (void)
{
  return (UART1Fifo.out_nchar);
}

inline int
UART1Fifo_in_get_nchar (void)
{
  return (UART1Fifo.in_nchar);
}



inline void
UART1SendTrigger (void)
{
  INTEnable (INT_SOURCE_UART_TX (UART1), INT_ENABLED);
}


void
UART1Send (const uint8_t * buffer, UINT32 size)
{
  while (size)
    {
      while (UART1Fifo.out_nchar > (UART1Fifo.bufsize - 10));
      ToUART1Fifo_out (*buffer);
      buffer++;
      size--;
    }
  UART1SendTrigger ();
}

void
UART1SendChar (const uint8_t character)
{

  ToUART1Fifo_out (character);
  UART1SendTrigger ();
}

void
UART1PutStr (const char *buffer)
{
  UART1Send ((uint8_t *) buffer, strlen (buffer));
}

/*
void
__ISR (_UART1_VECTOR, IPL2SOFT)
IntUart1Handler (void)
*/
#define U_VECTOR        _UART_1_VECTOR
void
  __attribute__ ((nomips16, interrupt (ipl2),
		  vector (U_VECTOR))) uart1_interrupt (void)
{
  if (INTGetFlag (INT_SOURCE_UART_RX (UART1)))
    {

      if (U1STAbits.URXDA)
	{
	  ToUART1Fifo_in (U1RXREG);
	}
      INTClearFlag (INT_SOURCE_UART_RX (UART1));
    }

  if (INTGetFlag (INT_SOURCE_UART_TX (UART1)))
    {
      if (UART1Fifo.out_nchar == 0)
	{
	  INTEnable (INT_SOURCE_UART_TX (UART1), INT_DISABLED);
	}
      else
	{
	  while (UART1Fifo.out_nchar > 0)
	    {
	      if (U1STAbits.UTXBF)
		{
		  break;
		}
	      U1TXREG = (char) FromUART1Fifo_out ();
	    }
	}
      INTClearFlag (INT_SOURCE_UART_TX (UART1));
    }
}

void
UART1Init (uint32_t SystemClock)
{
#define GetPeripheralClock()            (SystemClock/(1 << OSCCONbits.PBDIV))

  UART1FifoInit ();
// Pinguino
//U1
// U1RX (P4)            34: SOSCO/RPA4/RA4/T1CK/CTED9
// U1TX (P3)            33: SOSCI/RPB4/RB4

//U2
//D0 (U2RX)             04: RPC8/PMA5/RC8
//D1 (U2TX)             05: RPC9/CTED7/PMA6/RC9

  U1RXR = 2;			// 6;
  RPB4R = 1;			///;2;   



  UARTConfigure (UART1, UART_ENABLE_PINS_TX_RX_ONLY | UART_ENABLE_HIGH_SPEED);
  UARTSetFifoMode (UART1,
		   UART_INTERRUPT_ON_TX_BUFFER_EMPTY |
		   UART_INTERRUPT_ON_RX_NOT_EMPTY);
  UARTSetLineControl (UART1,
		      UART_DATA_SIZE_8_BITS | UART_PARITY_NONE |
		      UART_STOP_BITS_1);

  UARTSetDataRate (UART1, GetPeripheralClock (), 921600);
  UARTEnable (UART1, UART_ENABLE_FLAGS (UART_PERIPHERAL | UART_RX | UART_TX));
  INTEnable (INT_SOURCE_UART_RX (UART1), INT_ENABLED);
  INTSetVectorPriority (INT_VECTOR_UART (UART1), INT_PRIORITY_LEVEL_2);
  INTSetVectorSubPriority (INT_VECTOR_UART (UART1), INT_SUB_PRIORITY_LEVEL_0);
}
