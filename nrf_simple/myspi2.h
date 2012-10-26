#ifndef MYSPI2_H_INCLUDED
#define MYSPI2_H_INCLUDED

#include <p32xxxx.h>
#include <plib.h>
#include <stdint.h>

#include "myspi2.h"


#define NRF_CE_LOW()	LATCCLR = _LATC_LATC3_MASK;
#define NRF_CE_HIGH()     LATCSET = _LATC_LATC3_MASK;

#define NRF_CS_LOW()	{\
                        LATCCLR = _LATC_LATC2_MASK; \
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                        __asm__ ("nop");\
                       }





#define NRF_CS_HIGH()	LATCSET = _LATC_LATC2_MASK;

#define NRF_POWER_OFF()	LATCCLR = _LATC_LATC4_MASK;
#define NRF_POWER_ON()	LATCSET = _LATC_LATC4_MASK;



void SPI2_init (void);

uint8_t SPI2_xmit (const uint8_t data);
void SPI2_transmit (const uint8_t * data, const uint32_t len);

#endif
