#ifndef _MYSPI_H_
#define _MYSPI_H_
#include <stdint.h>
#include <p32xxxx.h>
#include <plib.h>

/*



#define CSN_nRF LATCbits.LATC2
#define CS_nRF_LOW()    CSN_nRF = 0;
#define CS_nRF_HIGH()   CSN_nRF = 1; 

#define CE_nRF  LATCbits.LATC3
#define CE_nRF_LOW()    CE_nRF  = 0; 
#define CE_nRF_HIGH()   CE_nRF  = 1; 

*/
#define CS_nRF_LOW()	LATCCLR=_LATC_LATC2_MASK; 
#define CS_nRF_HIGH()   LATCSET=_LATC_LATC2_MASK;

#define CE_nRF_LOW()    LATCCLR=_LATC_LATC3_MASK;
#define CE_nRF_HIGH()   LATCSET=_LATC_LATC3_MASK;


void    SPI1_init(void);
void	SPI1_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len);
void	SPI1_transmit_sync (const uint8_t * dataout, uint8_t len);
uint8_t SPI1_fast_shift (uint8_t data);

void	SPI2_init(void);
void	SPI2_transfer_sync (uint8_t * dataout, uint8_t * datain, uint8_t len);
void	SPI2_transmit_sync (const uint8_t * dataout, uint8_t len);
uint8_t SPI2_fast_shift (uint8_t data);
void 	SPI2_read (uint8_t * inBuf, uint8_t fillchar ,uint8_t length);

#endif /* _SPI_H_ */
