#ifndef SYSTICK_H__
#define SYSTICK_H__

#include <p32xxxx.h>
#include <plib.h>
#include <string.h>
#include <stdint.h>




void systick_init (void);

uint64_t get_systics (void);

#endif
