#include <stdint.h>

void uint32touint8p(unsigned int v, unsigned char  *p)
{
    *p++ = (v>>24)&0xFF;
    *p++ = (v>>16)&0xFF;
    *p++ = (v>> 8)&0xFF;
    *p++ = (v>> 0)&0xFF;
}

uint32_t uint8ptouint32(uint8_t  *p)
{
    uint32_t v=0;
    v |= *p++; v<<=8;
    v |= *p++; v<<=8;
    v |= *p++; v<<=8;
    v |= *p;
    return v;
}
