#include <p32xxxx.h>
#include <plib.h>
#include <string.h>
#include <stdint.h>


#define SystemClock()                        (40000000ul)
#define GetPeripheralClock()            (SystemClock()/(1 << OSCCONbits.PBDIV))
#define GetInstructionClock()           (SystemClock())

#define mLED_1              LATBbits.LATB15
#define mLED_2              LATAbits.LATA10

#define mGetLED_1()         mLED_1
#define mGetLED_2()         mLED_2

#define mLED_1_On()         mLED_1 = 1;
#define mLED_2_On()         mLED_2 = 1;

#define mLED_1_Off()        mLED_1 = 0;
#define mLED_2_Off()        mLED_2 = 0;

#define mLED_1_Toggle()     mLED_1 = !mLED_1;
#define mLED_2_Toggle()     mLED_2 = !mLED_2;


const char mainMenu[] =
  { "Welcome to PIC32 UART Peripheral Library Demo!\r\n" };


struct UARTFifo
{
  int in_read_pos;
  int in_write_pos;
  int out_read_pos;
  int out_write_pos;
  unsigned int in_nchar;
  unsigned int out_nchar;
  int bufsize;
  unsigned char *in;
  unsigned char *out;

} volatile UART2Fifo;


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
  UART2Fifo.bufsize = 100;

  UART2Fifo.in = malloc (UART2Fifo.bufsize);
  UART2Fifo.out = malloc (UART2Fifo.bufsize);
}


// Add one character to output fifo 
void
ToUART2Fifo_in (const char character)
{
  UART2Fifo.in[UART2Fifo.in_write_pos] = character;
  UART2Fifo.in_write_pos = (UART2Fifo.in_write_pos + 1) % UART2Fifo.bufsize;
  UART2Fifo.in_nchar++;
}

void
ToUART2Fifo_out (const char character)
{
  UART2Fifo.out[UART2Fifo.out_write_pos] = character;
  UART2Fifo.out_write_pos = (UART2Fifo.out_write_pos + 1) % UART2Fifo.bufsize;
  UART2Fifo.out_nchar++;
}

int
FromUART2Fifo_in (void)
{
  int in = -1;
  if (UART2Fifo.in_nchar > 0)
    {
      in = UART2Fifo.in[UART2Fifo.in_read_pos];
      UART2Fifo.in_read_pos = (UART2Fifo.in_read_pos + 1) % UART2Fifo.bufsize;
      UART2Fifo.in_nchar--;
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
      UART2Fifo.out_read_pos =
	(UART2Fifo.out_read_pos + 1) % UART2Fifo.bufsize;
      UART2Fifo.out_nchar--;
    }
  return (in);
}

inline int
UART2Fifo_out_get_nchar (void)
{
  return (UART2Fifo.out_nchar);
}

inline int
UART2Fifo_in_get_nchar (void)
{
  return (UART2Fifo.in_nchar);
}



inline void
UART2SendTrigger (void)
{
  INTEnable (INT_SOURCE_UART_TX (UART2), INT_ENABLED);
}

void
UART2Send (const char *buffer, UINT32 size)
{
  while (size)
    {
      ToUART2Fifo_out (*buffer);
      buffer++;
      size--;
    }
  UART2SendTrigger ();
}

void
UART2SendChar (const char character)
{

  ToUART2Fifo_out (character);
  UART2SendTrigger ();
}

void
UART2PutStr (const char *buffer)
{
  UART2Send (buffer, strlen (buffer));
}


void
__ISR (_UART2_VECTOR, IPL2SOFT)
IntUart2Handler (void)
{
  if (INTGetFlag (INT_SOURCE_UART_RX (UART2)))
    {

      if (U2STAbits.URXDA)
	{
	  ToUART2Fifo_in (U2RXREG);
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

int
main (void)
{

  UART2FifoInit ();
  int ii = 0;
  /* Configure PB frequency and wait states */
  SYSTEMConfigPerformance (40000000L);
  RPC9R = 2;
  U2RXR = 6;
//  TRISCbits.TRISC8 = 1;
//  TRISCbits.TRISC9 = 0;
  UARTConfigure (UART2, UART_ENABLE_PINS_TX_RX_ONLY | UART_ENABLE_HIGH_SPEED);
  UARTSetFifoMode (UART2,
		   UART_INTERRUPT_ON_TX_BUFFER_EMPTY |
		   UART_INTERRUPT_ON_RX_NOT_EMPTY);
  UARTSetLineControl (UART2,
		      UART_DATA_SIZE_8_BITS | UART_PARITY_NONE
		      | UART_STOP_BITS_1);
  UARTSetDataRate (UART2, GetPeripheralClock (), 500000);
  UARTEnable (UART2, UART_ENABLE_FLAGS (UART_PERIPHERAL | UART_RX | UART_TX));
  // Configure UART2 RX Interrupt
  INTEnable (INT_SOURCE_UART_RX (UART2), INT_ENABLED);
//  INTEnable (INT_SOURCE_UART_TX (UART2), INT_ENABLED);
  INTSetVectorPriority (INT_VECTOR_UART (UART2), INT_PRIORITY_LEVEL_2);
  INTSetVectorSubPriority (INT_VECTOR_UART (UART2), INT_SUB_PRIORITY_LEVEL_0);
  // configure for multi-vectored mode
  INTConfigureSystem (INT_SYSTEM_CONFIG_MULT_VECTOR);
  // enable interrupts
  INTEnableInterrupts ();
  LATAbits.LATA10 = 0;
  TRISAbits.TRISA10 = 0;
  LATBbits.LATB15 = 0;
  TRISBbits.TRISB15 = 0;
//  mPORTASetPinsDigitalOut (BIT_10);
#define DELAY 400		//10156
  T1CON = 0x8030;
  PR1 = 0xffff;
  TMR1 = 0;
  while (TMR1 < DELAY);
  UART2PutStr ("\r\r\n\n\n\rHallo NOKLAB\r\n");
#define DELAYA 1210
  T1CON = 0x8030;
  PR1 = 0xffff;
  TMR1 = 0;
  while (TMR1 < DELAYA);
  UART2PutStr ("1111111111111111\r\n");
  UART2PutStr ("2222222222222222\r\n");
  T1CON = 0x8030;
  PR1 = 0xffff;
  TMR1 = 0;
  while (TMR1 < DELAY);
  while (1)
    {
      char buf[10];
//      int  tmp;
      //UART2SendChar (UART2Fifo.out_nchar + '0');
      while (UART2Fifo_out_get_nchar () != 0);
/////////////
/*
      while ( 1 )
        {
        tmp = FromUART2Fifo_in();
        if ( tmp < 0 )
          break;
        inBuf[inBufCnt] = (unsigned char) tmp;

        if (inBuf[inBufCnt] == '\r')
          {
          inBuf[++inBufCnt] = 0;
          UART2PutStr(inBuf);
          inBufCnt = 0;
          }
        else
          inBufCnt++;  
        
        break;  
        }
*/
//////////////
      ultoa (buf, ii++, 10);
      UART2PutStr (buf);
      UART2PutStr (" ");
      UART2PutStr ("\n\r");
      T1CON = 0x8030;
      PR1 = 0xffff;
      TMR1 = 0;
      T1CON = 0x8030;
      PR1 = 0xffff;
      TMR1 = 0;
//while (TMR1 < DELAYA);
    }
}
