#ifndef UART2_H__
#define UART2_H__

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

inline int UART1ReadChar(void);


inline int
UART1Fifo_out_get_nchar (void);

inline int
UART1Fifo_in_get_nchar (void);

void 
UART1Send (const char *buffer, UINT32 size);

void 
UART1SendChar (const char character);

void 
UART1PutStr (const char *buffer);

void
UART1Init (uint32_t SystemClock);

inline int UART2ReadChar(void);

inline int
UART2Fifo_out_get_nchar (void);

inline int
UART2Fifo_in_get_nchar (void);

void 
UART2Send (const char *buffer, UINT32 size);

void 
UART2SendChar (const char character);

void 
UART2PutStr (const char *buffer);

void
UART2Init (uint32_t SystemClock);



#endif