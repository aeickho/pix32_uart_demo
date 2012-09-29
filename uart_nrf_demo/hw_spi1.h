#ifndef _FLAUSCH_SPI_H
#define _FLAUSCH_SPI_H

#define SD_CS_TRIS              TRISAbits.TRISA7
#define SD_CS                   LATAbits.LATA7  
#define mPin_Config_SPI1_RX()   SDI1R = 5; // MISO / RX
#define mPin_Config_SPI1_TX()   RPA9R = 3; // MOSI / TX
#define SD_SPI_BRG              SPI1BRG
#define SD_SPI_CON              SPI1CON
#define SD_SPI_BUF              SPI1BUF
#define SD_SPI_STATbits         SPI1STATbits



void spi_sd_init();
unsigned char spi_sd_xmit(const unsigned char in);
void sd_init();

#endif // _FLAUSCH_SPI_H
