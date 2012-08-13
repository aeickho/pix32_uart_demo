#include "../spi.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "mega48.h"

void
wl_module_init ()
// Initializes pins and interrupt to communicate with the wl_module
// Should be called in the early initializing phase at startup.
{
  // Define CSN and CE as Output and set them to default
  DDRB |= ((1 << CSN) | (1 << CE));
  CE_LOW ();
  CS_HIGH ();

#if defined(__AVR_ATmega48__)
  EICRA = ((1 << ISC11) | (0 << ISC10) | (1 << ISC01) | (0 << ISC00));	// Set external interupt on falling edge for INT0 and INT1
  EIMSK = ((0 << INT1) | (1 << INT0));	// Activate INT0
#endif // __AVR_ATmega88A__

  // Initialize spi module
  spi_init ();
}



void
sspInit (uint8_t portNum, sspClockPolarity_t polarity, sspClockPhase_t phase)
{
  portNum--;
  wl_module_init ();
}

void
sspSend (uint8_t portNum, const uint8_t * buf, uint32_t length)
{
  portNum--;
  spi_transmit_sync ( buf, length);
}

void
sspReceive (uint8_t portNum, uint8_t * buf, uint32_t length)
{
  portNum--;
  spi_transfer_sync (buf, buf, length);
}

void
sspSendReceive (uint8_t portNum, uint8_t * buf, uint32_t length)
{
  portNum--;
  spi_transfer_sync (buf, buf, length);
}
