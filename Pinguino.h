#ifndef _PINGUINO_H_
#define _PINGUINO_H_


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


void delay_1ms (void);
void _delay_ms (unsigned int length);
            
#endif
