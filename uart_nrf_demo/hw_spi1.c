#include "hw_spi1.h"
#include <p32xxxx.h>
#include <plib.h>


void spi_sd_init() {
	// set up CS as output
	SD_CS_TRIS = 0;
	SD_CS = 1; // set CS high

	// disable A/D units
	// TODO: factor out config to platform_config.h
	ANSELA = 0; ANSELB = 0; ANSELC = 0;

	// set up MOSI/MISO pins
	mPin_Config_SPI1_RX();
	mPin_Config_SPI1_TX();

	//SPI1STATCLR = 1<<6; // clear SPIROV (not needed)

	SD_SPI_BRG = 85; /* 232.56 kHz */

	SD_SPI_CON = 0x8120; /* ON=1, MODE32=0, MODE16=0, CKE=1, MSTEN=1 */
}

unsigned char spi_sd_xmit(const unsigned char in) {
	SD_SPI_BUF = in;
	while(!SD_SPI_STATbits.SPIRBF);
	return SD_SPI_BUF;
}

