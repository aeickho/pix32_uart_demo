#ifndef RXMSG__H
#define RXMSG__H


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aeickho-tiny-fecc/tfec3.h"



void rxmsg_process_frame (uint8_t * inData);
int rxmsg_get_frame( uint8_t * outData);
#endif
