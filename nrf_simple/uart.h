#ifndef AE_UART_H__
#define AE_UART_H__

#include <p32xxxx.h>
#include <plib.h>
#include <string.h>
#include <stdint.h>



struct UARTFifo
{
  uint16_t in_read_pos;
  uint16_t in_write_pos;
  uint16_t out_read_pos;
  uint16_t out_write_pos;
  uint16_t in_nchar;
  uint16_t out_nchar;
  uint16_t bufsize;
  uint8_t *in;
  uint8_t *out;

};

/*
void UART1Init (uint32_t SystemClock);

int UART1ReadChar (void);

void UART1Read (uint8_t * buf, const uint16_t n);

void UART1Send (const uint8_t * buffer, UINT32 size);

void UART1SendChar (const uint8_t character);

void UART1PutStr (const char *buffer);

int UART1Fifo_out_get_nchar (void);

int UART1Fifo_in_get_nchar (void);

void UART1PutHexChar (const int val);
void UART1PutHex (unsigned int val);

*/

int UART2ReadChar (void);

void UART2Read (uint8_t * buf, const uint16_t n);

int UART2Fifo_out_get_nchar (void);

int UART2Fifo_in_get_nchar (void);

void UART2Send (const uint8_t * buffer, UINT32 size);

void UART2SendChar (const uint8_t character);

void UART2PutStr (const char *buffer);

void UART2Init (uint32_t SystemClock);

void UART2PutHexChar (const int val);
void UART2PutHex (unsigned int val);
void UART2PutCharHex (unsigned char val);

#endif
