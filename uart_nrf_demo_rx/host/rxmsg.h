#ifndef RXMSG__H
#define RXMSG__H


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aeickho-tiny-fecc/tfec3.h"



int rxmsg_process_frame (uint8_t * inData, uint8_t * outData);

#endif
