#include <p32xxxx.h>
#include <plib.h>
#include <string.h>
#include <stdint.h>
#include "uart2.h"

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


int
main (void)
{
  int ii;

  SYSTEMConfigPerformance (SystemClock());


  UART2Init (SystemClock());

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
#define DELAYA 300
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
while (TMR1 < DELAYA);
    }
}
