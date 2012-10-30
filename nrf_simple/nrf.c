#include <plib.h>
#include <stdint.h>

#include "myspi2.h"
#include "nrf.h"
#include "general_exception_handler.h"
#include "delay.h"
#include "uart.h"
#include "portsetup.h"


uint8_t
nrf_read_reg (const uint8_t reg)
{
  uint8_t val;
  NRF_CS_LOW ();
/*  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
  __asm__ ("nop");
*/
  SPI2_xmit (C_R_REGISTER | reg);
  val = SPI2_xmit (0xff);
  NRF_CS_HIGH ();
  return val;
};


void
nrf_write_reg (const uint8_t reg, const uint8_t val)
{
  NRF_CS_LOW ();
  SPI2_xmit (C_W_REGISTER | reg);
  SPI2_xmit (val);
  NRF_CS_HIGH ();
}

void
nrf_write_long (const uint8_t cmd, int len, const uint8_t * data)
{
  NRF_CS_LOW ();
  SPI2_xmit (cmd);
  SPI2_transmit (data, len);
  NRF_CS_HIGH ();
}

#define nrf_write_reg_long(reg, len, data) \
    nrf_write_long(C_W_REGISTER|(reg), len, data)


uint8_t
nrf_cmd_status (uint8_t data)
{
  NRF_CS_LOW ();
  data = SPI2_xmit (data);
  NRF_CS_HIGH ();
  return data;
}


void
nrf_reset (void)
{
  NRF_POWER_OFF ();
  NRF_CS_HIGH ();
  NRF_CE_LOW ();
  delay_ms (10);
  NRF_POWER_ON ();
  delay_ms (100);		// see nRF24L01+ Product Specification Page 22
}


void
nrf_init (void)
{
  SPI2_init ();
  nrf_reset ();

  nrf_write_reg (R_CONFIG,
		 R_CONFIG_PRIM_RX | R_CONFIG_PWR_UP | R_CONFIG_EN_CRC);
  nrf_write_reg (R_EN_AA, 0);
  nrf_write_reg (R_RF_SETUP, R_RF_SETUP_DR_2M | R_RF_SETUP_RF_PWR_3);
  nrf_write_reg (R_STATUS, R_STATUS_MAX_RT);
}


void
nrf_config_set (nrfconfig config)
{

  int i;
  nrf_write_reg (R_SETUP_AW, R_SETUP_AW_5);
  nrf_write_reg (R_RF_CH, config->channel);


  for (i = 0; i < config->nrmacs; i++)
    {
      nrf_write_reg (R_RX_PW_P0 + i, config->maclen[i]);
      if (i == 0)
	{
	  nrf_write_reg_long (R_RX_ADDR_P0, 5, config->mac0);
	}
      else if (i == 1)
	{
	  nrf_write_reg_long (R_RX_ADDR_P1, 5, config->mac1);
	}
      else if (i > 1)
	{
	  nrf_write_reg_long (R_RX_ADDR_P0 + i, 1, config->mac2345 + i - 2);
	};
    };
  nrf_write_reg_long (R_TX_ADDR, 5, config->txmac);
  nrf_write_reg (R_EN_RXADDR, (1 << config->nrmacs) - 1);
}

// Diese Funktion geht davon aus:
// - das nichts mehr im Send_fifo ist


void
nrf_send_frame (uint8_t * frame, int mode)
{
  int ret;

  nrf_write_reg (R_CONFIG, R_CONFIG_PWR_UP | R_CONFIG_EN_CRC);

  NRF_CS_LOW ();
  mLED_2_On ();
  SPI2_xmit (C_W_TX_PAYLOAD);
  SPI2_transmit (frame, 32);
  mLED_2_Off ();
  NRF_CS_HIGH ();

  NRF_CE_HIGH ();
  while (1)
    {
      ret = nrf_read_reg (R_FIFO_STATUS);
      if (mode)
	{
	  if ((ret & R_FIFO_STATUS_TX_EMPTY) == R_FIFO_STATUS_TX_EMPTY)
	    break;
	}
      else
	{
	  if ((ret & R_FIFO_STATUS_TX_FULL) == 0)
	    break;
	}
    }
  NRF_CE_LOW ();

/*  nrf_write_reg (R_STATUS,
		 R_CONFIG_MASK_RX_DR | R_CONFIG_MASK_TX_DS |
		 R_CONFIG_MASK_MAX_RT);

  ret = nrf_cmd_status (C_NOP);
*/

}


