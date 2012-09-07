#ifndef _PINGUINO_H_
#define _PINGUINO_H_

#define mGetLED_1()         LATBbits.LATB15
#define mGetLED_2()         LATAbits.LATA10

#define mLED_1_On()         LATBSET = _LATB_LATB15_MASK; //mLED_1 = 1;
#define mLED_2_On()         LATASET = _LATA_LATA10_MASK;// mLED_2_bit; //mLED_2

#define mLED_1_Off()        LATBCLR = _LATB_LATB15_MASK; //mLED_1_bit; //mLED_1
#define mLED_2_Off()        LATACLR = _LATA_LATA10_MASK; //mLED_2_bit; //mLED_2

#define mLED_1_Toggle()     LATBINV = _LATB_LATB15_MASK;
#define mLED_2_Toggle()     LATAINV = _LATA_LATA10_MASK; //!mLED_2;



#define CE  LATCbits.LATC3
#define CSN LATCbits.LATC2


#define CS_LOW()    CSN = 0;
#define CS_HIGH()   CSN = 1; 
#define CE_LOW()    CE  = 0; 
#define CE_HIGH()   CE  = 1; 



void sspInit (unsigned char portNum, unsigned int dummy1,
	      unsigned int dummy2);
void sspSend (unsigned char portNum, const unsigned char *buf,
	      unsigned int length);
void sspReceive (unsigned char portNum, unsigned char *buf,
		 unsigned int length);
void sspSendReceive (unsigned char portNum, unsigned char *buf,
		     unsigned int length);



void SendDataBuffer (const char *buffer, UINT32 size);
void UART2Out (const char *buffer);

void delay_7us(void);
void delay_1ms (void);
void _delay_ms (unsigned int length);
            
#endif
