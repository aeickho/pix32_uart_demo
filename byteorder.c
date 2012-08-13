#include <stdint.h>

void uint32touint8p(unsigned int v, unsigned char  *p)
{
    *p++ = (v>>24)&0xFF;
    *p++ = (v>>16)&0xFF;
    *p++ = (v>> 8)&0xFF;
    *p++ = (v>> 0)&0xFF;
}

unsigned int uint8ptouint32(unsigned char  *p)
{
    uint32_t v=0;
    v |= *p++; v<<=8;
    v |= *p++; v<<=8;
    v |= *p++; v<<=8;
    v |= *p;
    return v;
}
