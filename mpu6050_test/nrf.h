#ifndef NRF_H_INCLUDED
#define NRF_H_INCLUDED

#include "nrf24l01p_reg.h"

void nrf_reset ();
void nrf_init (void);
void nrf_config_set (nrfconfig config);

void nrf_send_frame (uint8_t * frame, int mode);

void nrf_receive_start (void);
int nrf_receive_poll (uint8_t * frame);
void nrf_receive_stop (void);

void nrf_bg_receive_start (void);
void nrf_bg_receive_stop (void);

#endif
