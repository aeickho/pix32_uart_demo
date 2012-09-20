#ifndef _MYSPI_H_
#define _MYSPI_H_
#include <stdint.h>


void	SPI1_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len);
void	SPI1_transmit_sync (const uint8_t * dataout, uint8_t len);
uint8_t SPI1_fast_shift (uint8_t data);

void	SPI2_init(void);
void	SPI2_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len);
void	SPI2_transmit_sync (const uint8_t * dataout, uint8_t len);
uint8_t SPI2_fast_shift (uint8_t data);

#endif /* _SPI_H_ */
