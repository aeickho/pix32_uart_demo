#ifndef _BYTEORDER_H_
#define _BYTEORDER_H_

#include <stdint.h>

void uint32touint8p(unsigned int v, unsigned char  *p);
uint32_t int8ptouint32(uint8_t *p);

#endif
