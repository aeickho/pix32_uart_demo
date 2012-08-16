#ifndef _OPENBEACON_H_
#define _OPENBEACON_H_
#include <stdint.h>
#include "openbeacon.h"
#include "nrf24l01p.h"

#define OPENBEACON_SAVE 0xFFFF
#define OPENBEACON_CHANNEL 81

void openbeaconSetup(void);
void openbeaconSend(void);

#endif
