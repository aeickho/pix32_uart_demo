#ifndef _SPI_H_
#define _SPI_H_

void	spi_init();
void	spi_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len);
void	spi_transmit_sync (const uint8_t * dataout, uint8_t len);
uint8_t spi_fast_shift (uint8_t data);

#endif /* _SPI_H_ */
