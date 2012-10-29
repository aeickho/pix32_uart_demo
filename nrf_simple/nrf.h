#ifndef NRF_H_INCLUDED
#define NRF_H_INCLUDED

#include "nrf24l01p_reg.h"

void nrf_reset ();
void nrf_init (void);
void nrf_config_set (nrfconfig config);

void nrf_send_frame (uint8_t * frame, int mode);
//void nrf_send_frames (uint8_t * frame, int frames);



#endif
