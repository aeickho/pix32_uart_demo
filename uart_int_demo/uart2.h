#ifndef UART2_H__
#define UART2_H__

#include <p32xxxx.h>
#include <plib.h>
#include <string.h>
#include <stdint.h>

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