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
#include "myspi.h"
#include "basic.h"
#include "byteorder.h"

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


#define MAXPACKET   32
void
rftransfer_send (uint16_t size, uint8_t * data)
{
  uint8_t mac[5] = { 1, 2, 3, 2, 1 };
  struct NRF_CFG config = {
    .channel = 81,
    .txmac = "\x1\x2\x3\x2\x1",
    .nrmacs = 1,
    .mac0 = "\x1\x2\x3\x2\x1",
    .maclen = "\x20",
  };

  UART2Out ("rftransfer_send\n\r");


  static struct NRF_CFG oldconfig;
  nrf_config_get (&oldconfig);

  nrf_set_channel (81);
  nrf_set_strength (3);
  const uint8_t macx[5] = { 1, 2, 3, 2, 1 };
  nrf_set_tx_mac (sizeof (macx), macx);



  uint8_t buf[MAXPACKET];
  buf[0] = 'L';
  buf[1] = size >> 8;
  buf[2] = size & 0xFF;

//  uint16_t rand = getRandom () & 0xFFFF;
  uint16_t rand = 0x5555;

  buf[3] = rand >> 8;
  buf[4] = rand & 0xFF;

//  nrf_config_set (&config);
  nrf_snd_pkt_crc (32, buf);	//setup packet
  _delay_ms (20);
  uint16_t index = 0;
  uint8_t i;
  uint16_t crc = crc16 (data, size);

  while (size)
    {
      buf[0] = 'D';
      buf[1] = index >> 8;
      buf[2] = index & 0xFF;
      buf[3] = rand >> 8;
      buf[4] = rand & 0xFF;
      for (i = 5; i < MAXPACKET - 2 && size > 0; i++, size--)
	{
	  buf[i] = *data++;
	}
      index++;
//      nrf_config_set (&config);
      nrf_snd_pkt_crc (32, buf);	//data packet
      _delay_ms (20);
    }

  buf[0] = 'C';
  buf[1] = crc >> 8;
  buf[2] = crc & 0xFF;
  buf[3] = rand >> 8;
  buf[4] = rand & 0xFF;
//  nrf_config_set (&config);
  nrf_snd_pkt_crc (32, buf);	//setup packet
  _delay_ms (20);

  nrf_config_set (&oldconfig);

}



int
main (void)
{
  struct NRF_CFG config;
  uint8_t buf[32];
  uint8_t outBuf[32];
  uint16_t cnt;

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

  UART2Out
    (".............................................................................hallo\r\n");
  UART2Out ("Welt\r\n");

  UART2Out ("nrf_init(),");
  nrf_init ();
  UART2Out ("done\n\r");

  UART2Out ("openbeaconSend(),");
  openbeaconSend ();
  UART2Out ("done\n\r");

  UART2Out ("nrf_config_set(),");
  config.nrmacs = 1;
  config.maclen[0] = 32;
  config.channel = 81;
  ultoa (buf, config.channel, 10);
  UART2Out ("cannel: ,");
  UART2Out (buf);
  memcpy (config.mac0, "\x01\x02\x03\x02\x01", 5);
  nrf_config_set (&config);
  UART2Out ("done\n\r");

  struct NRF_CFG oldconfig;

  struct NRF_CFG config1 = {
    .channel = 81,
    .txmac = "\x1\x2\x3\x2\x1",
    .nrmacs = 1,
    .mac0 = "\x1\x2\x3\x2\x1",
    .maclen = "\x20",
  };

  nrf_config_get (&oldconfig);
  nrf_config_set (&config1);


  buf[0] = 32;
 
  for (c=1;c<32;c++)
  buf[c] = c;

  UART2Out ("read:\n\r");
  cnt = 0;
  do
    {

      cnt++;


      buf[2] = cnt >> 8;
      buf[3] = cnt & 0xff;
      mLED_2_On ();

      nrf_snd_pkt_crc (32, buf);
      mLED_2_Off ();

//      nrf_config_set (&oldconfig);
//      nrf_set_strength (3);
_delay_ms(10); 
    }
  while (1);

  return 0;
}
