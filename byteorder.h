#ifndef _BYTEORDER_H_
#define _BYTEORDER_H_

#include <stdint.h>

void uint32touint8p(unsigned int v, unsigned char  *p);
unsigned int uint8ptouint32(unsigned char *p);

#endif
