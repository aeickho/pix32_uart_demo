#ifndef AE_BASE128_H
#define AE_BASE128_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>


void to_base128 (const uint8_t * in, uint8_t * out);
void from_base128 (const uint8_t * in, uint8_t * out);
void to_base128n  (const uint8_t * in, uint8_t * out, uint8_t n);
void from_base128n(const uint8_t * in, uint8_t * out, uint8_t n);


#endif