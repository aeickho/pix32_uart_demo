#ifndef _SPI_H_
#define _SPI_H_

extern void spi_init();
extern void spi_transfer_sync (unsigned char * dataout, unsigned char * datain, unsigned char len);
extern void spi_transmit_sync (const unsigned char * dataout, unsigned char len);
extern unsigned char spi_fast_shift (unsigned char data);

#endif /* _SPI_H_ */
