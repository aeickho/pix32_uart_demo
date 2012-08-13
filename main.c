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

#include "Pinguino.h"
#include "nrf24l01p.h"
#include "openbeacon.h"
#include "spi.h"

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



int
main (void)
{
  struct NRF_CFG config;
  unsigned char buf[16];
  char outBuf[16];
  unsigned char cnt;

  int c;

  /* Configure PB frequency and wait states */
  SYSTEMConfigPerformance (40000000L);


// UART
  U2RXR = 6;			// UART2
  RPC9R = 2;			// UART2 

//  TRISCbits.TRISC8 = 1;               // UART2 RX as Input
//  TRISCbits.TRISC9 = 0;               // UART2 TX as Output

// LED2
  LATAbits.LATA10 = 0;		// LED2
  TRISAbits.TRISA10 = 0;

// SPI2
  TRISBbits.TRISB5 = 0;		// SD02 as output
  RPB5R = 4;			// SDO2

  TRISBbits.TRISB13 = 1;	// SDI2 as input
  SDI2R = 3;			// RPB13;

  TRISBbits.TRISB15 = 0;	// SCK2 as output  (fixed pin)
  ANSELA = 0;
  ANSELB = 0;
  ANSELC = 0;

// CS// CE
  TRISCbits.TRISC2 = 0;		// CSN as output
  TRISCbits.TRISC3 = 0;		// CE as output

  CS_HIGH ();			// NO SPI Chip Select
  CE_LOW ();			// NO Chip Enable Activates RX or TX mode

/*  SPI2CON = 0;
  SPI2CONbits.MSTEN = 1;
  SPI2CONbits.CKE = 1;
//  SPI2CONbits.SMP = 1; 
  SPI2BRG = 1;
  SPI2CON2 = 0;
  SPI2CONbits.ON = 1;
*/
/*
 OpenSPI2( SPI_MODE8_ON | MASTER_ENABLE_ON | SEC_PRESCAL_1_1 | PRI_PRESCAL_1_1 | FRAME_ENABLE_OFF | CLK_POL_ACTIVE_HIGH | ENABLE_SDO_PIN , SPI_ENABLE );
 SPI2BRG = 1;
*/
  SPI2BRG = 4;
  SPI2STATCLR = 1 << 6;		// clear SPIROV  
  SPI2CON = 0x8120;


// config UART2    
  UARTConfigure (UART2, UART_ENABLE_PINS_TX_RX_ONLY | UART_ENABLE_HIGH_SPEED);
  UARTSetFifoMode (UART2,
		   UART_INTERRUPT_ON_TX_NOT_FULL |
		   UART_INTERRUPT_ON_RX_NOT_EMPTY);
  UARTSetLineControl (UART2,
		      UART_DATA_SIZE_8_BITS | UART_PARITY_NONE |
		      UART_STOP_BITS_1);

  UARTSetDataRate (UART2, GetPeripheralClock (), 500000);


  UARTEnable (UART2, UART_ENABLE_FLAGS (UART_PERIPHERAL | UART_RX | UART_TX));









  _delay_ms (10);

  UART2Out
    (".............................................................................hallo\r\n");
  UART2Out ("Welt\r\n");




/* while (1)
{
CS_LOW();
spi_transmit_sync ("x", 1);
CS_HIGH();
_nop();
_nop();
_nop();
_nop();
_nop();

_nop();
_nop();
_nop();
_nop();
_nop();

_nop();
_nop();
_nop();
_nop();
_nop();
}
*/

   










  UART2Out ("nrf_init(),");
  nrf_init ();
  UART2Out ("done\n\r");

  UART2Out ("openbeaconSend(),");
  openbeaconSend ();
  UART2Out ("done\n\r");

  UART2Out ("nrf_config_set(),");
  config.nrmacs = 1;
  config.maclen[0] = 16;
  config.channel = 81;
  ultoa (buf, config.channel, 10);
  UART2Out ("cannel: ,");
  UART2Out (buf);
  memcpy (config.mac0, "\x01\x02\x03\x02\x01", 5);
  nrf_config_set (&config);
  UART2Out ("done\n\r");

  UART2Out ("read:\n\r");
  cnt = 0;
  do
    {
//     UART2Out ("w:");
/*
      if (cnt++ > 50)
	{
	  cnt = 0;
	  UART2Out ("openbeaconSend(),");
	  openbeaconSend ();
	  UART2Out ("done\n\r");
//  delay_1ms (20);

//        nrf_config_set(&config);

	}
*/
      if (nrf_rcv_pkt_time (64, sizeof (buf), buf) == 16)
	{
	  uint32_t i;
	  ultoa (outBuf, buf[1], 16);
	  UART2Out (outBuf);
	  UART2Out (" ");


	  buf[14] = 0;
	  if (buf[1] == 0x23 || buf[1] == 0x24)
	    {
	      i = uint8ptouint32 (buf + 2);
	      //sprintf(outBuf, "%x",i);
	      ultoa (outBuf, i, 16);
	      UART2Out (outBuf);
	      UART2Out (" ");

	      UART2Out ((char *) (buf + 6));
	      UART2Out ("\n\r");
	    }
	  if (buf[1] == 0x17)
	    {
	      char b[10];
	      i = uint8ptouint32 (buf + 8);
	      //sprintf(outBuf, "%x",i);
	      ultoa (outBuf, i, 16);
	      UART2Out (outBuf);
	      UART2Out (" ");

	      i = uint8ptouint32 (buf + 4);
	      ultoa (outBuf, i, 10);
	      UART2Out (outBuf);
	      UART2Out (" ");
	      i = buf[3] / 85;
	      ultoa (outBuf, i, 10);
	      UART2Out (outBuf);

	      UART2Out ("\n\r");

	    }

	}
    }
  while (1);

  return 0;
}
