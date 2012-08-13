#include <avr/io.h>
#include <util/delay.h>

/**************************************************************************/
/*! 
    Indicates whether the clock should be high or low between frames.
    */
    /**************************************************************************/
typedef enum sspClockPolarity_e
{
  sspClockPolarity_Low = 0,
  sspClockPolarity_High
}
sspClockPolarity_t;

	/**************************************************************************/
	/*! 
	   Indicates whether the bits start at the rising or falling edge of
	   the clock transition.
	 */
		/**************************************************************************/
typedef enum sspClockPhase_e
{
  sspClockPhase_RisingEdge = 0,
  sspClockPhase_FallingEdge
}
sspClockPhase_t;



#define CE  PB0
#define CSN PB1


#define CS_LOW()    PORTB &= ~(1<<CSN); //// gpioSetValue(RB_SPI_NRF_CS, 0)
#define CS_HIGH()   PORTB |=  (1<<CSN); ////gpioSetValue(RB_SPI_NRF_CS, 1)
#define CE_LOW()    PORTB &= ~(1<<CE);  ////gpioSetValue(RB_NRF_CE, 0)
#define CE_HIGH()   PORTB |=  (1<<CE);  /////gpioSetValue(RB_NRF_CE, 1)


void sspInit (uint8_t portNum, sspClockPolarity_t polarity, sspClockPhase_t phase);
void sspSend (uint8_t portNum, const uint8_t *buf, uint32_t length);
void sspReceive (uint8_t portNum, uint8_t *buf, uint32_t length);
void sspSendReceive(uint8_t portNum, uint8_t *buf, uint32_t length);

