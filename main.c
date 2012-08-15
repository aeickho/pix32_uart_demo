/*
	Example "blinkenlights" program for the UBW32 and C32 compiler.
	Demonstrates software PWM, use of floating-point library, etc.

	IMPORTANT: file 'procdefs.ld' absolutely must Must MUST be present
	in the working directory when compiling code for the UBW32!  Failure
	to do so will almost certainly result in your UBW32 getting 'bricked'
	and requiring re-flashing the bootloader (which, if you don't have a
	PICkit 2 or similar PIC programmer, you're screwed).
	YOU HAVE BEEN WARNED.

	2/19/2009 - Phillip Burgess - pburgess@dslextreme.com
*/

#include <p32xxxx.h>
#include <plib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


#define SystemClock()                        (40000000ul)
#define GetPeripheralClock()            (SystemClock()/(1 << OSCCONbits.PBDIV))
#define GetInstructionClock()           (SystemClock())



//#define mLED_1              LATBbits.LATB15
#define mLED_2              LATAbits.LATA10

//#define mGetLED_1()         mLED_1
#define mGetLED_2()         mLED_2

//#define mLED_1_On()         mLED_1 = 1;
#define mLED_2_On()         mLED_2 = 1;

//#define mLED_1_Off()        mLED_1 = 0;
#define mLED_2_Off()        mLED_2 = 0;

//#define mLED_1_Toggle()     mLED_1 = !mLED_1;
#define mLED_2_Toggle()     mLED_2 = !mLED_2;



const char mainMenu[] =
  { "Welcome to PIC32 UART Peripheral Library Demo +  nRF24L01+ !\r\n" };


SendDataBuffer (const char *buffer, UINT32 size)
{
  while (size)
      {
            while (!UARTTransmitterIsReady (UART2))
                    ;
                    
                          UARTSendDataByte (UART2, *buffer);
                          
                                mPORTAToggleBits (BIT_10);
                                
                                      buffer++;
                                            size--;
                                                }
                                                
                                                //  while (!UARTTransmissionHasCompleted (UART2));
                                                }
                                                
                                                void
                                                UART2Out (const char *buffer)
                                                {
                                                  SendDataBuffer (buffer, strlen (buffer));
                                                  }
                                                  

int
main (void)
{
  SYSTEMConfigPerformance (40000000L);


// UART Init sample

U2RXR = 6;  // UART2  RPC8 / PMA5 / RC8         see PIC32 Family Reference Manual Section 12. I/O Ports, Table 12-1
RPC9R = 2;  // UART2  RPC9 / CTED7 / PMA6 / RC9 see PIC32 Family Reference Manual Section 12. I/O Ports, Table 12-2


UARTConfigure (UART2, UART_ENABLE_PINS_TX_RX_ONLY| UART_ENABLE_HIGH_SPEED);
UARTSetFifoMode (UART2,UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
UARTSetLineControl (UART2, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
UARTSetDataRate (UART2, GetPeripheralClock (), 300);
UARTEnable (UART2, UART_ENABLE_FLAGS (UART_PERIPHERAL | UART_RX | UART_TX));

UARTSendDataByte (UART2, 'A');

  while (1);
}
