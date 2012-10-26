#include <p32xxxx.h>
#include <plib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Pinguino.h"
#include "nrf24l01p.h"
#include "myspi.h"
#include "basic.h"
#include "byteorder.h"
#include "uart.h"
#include "portsetup.h"
#include "tfec3_tools.h"
#include "tools/printf.h"


#define SystemClock()                        (40000000ul)
#define GetPeripheralClock()            (SystemClock()/(1 << OSCCONbits.PBDIV))
#define GetInstructionClock()           (SystemClock())




#define MAXPACKET   32



////////////////
#define WORDS_PER_FRAGMENT   6
#define BYTES_PER_FRAGMENT  (WORDS_PER_FRAGMENT*4)




#define NRF_CE_LOW()    LATCCLR = _LATC_LATC3_MASK;
#define NRF_CE_HIGH()     LATCSET = _LATC_LATC3_MASK;

#define NRF_CS_LOW()    LATCCLR = _LATC_LATC2_MASK;
#define NRF_CS_HIGH()   LATCSET = _LATC_LATC2_MASK;

#define NRF_POWER_OFF() LATCCLR = _LATC_LATC4_MASK;
#define NRF_POWER_ON()  LATCSET = _LATC_LATC4_MASK;




uint8_t
SPI2_xmit (const uint8_t data)
{
  SPI2BUF = data;
  while (!SPI2STATbits.SPIRBF);
  return (SPI2BUF);
}

void
SPI2_transmit (const uint8_t * data, const uint32_t len)
{
  int i;

  for (i = 0; i < len; i++)
    SPI2_xmit (data[i]);
}



void
nrf_write_longX (const uint8_t cmd, int len, const uint8_t * data)
{
  NRF_CS_LOW ();
  SPI2_xmit (cmd);
  SPI2_transmit (data, len);
  NRF_CS_HIGH ();
}

#define nrf_write_reg_long(reg, len, data) \
             nrf_write_longX(C_W_REGISTER|(reg), len, data)



void
nrf_send_frame (uint8_t * frame)
{
  int ret;

  nrf_write_reg (R_CONFIG, R_CONFIG_PWR_UP | R_CONFIG_EN_CRC);


  for (ret = 0; ret < 32; ret++)
    {
      UART2PutHex (frame[ret]);
      UART2PutStr (" ");

    }
  UART2PutStr ("\n\r");

  NRF_CS_LOW ();
  SPI2_xmit (C_W_TX_PAYLOAD);
  SPI2_transmit (frame, 32);
  NRF_CS_HIGH ();

  NRF_CE_HIGH ();
  while (1)
    {
      ret = nrf_read_reg (R_FIFO_STATUS);
      if ((ret & R_FIFO_STATUS_TX_EMPTY) == R_FIFO_STATUS_TX_EMPTY)
	break;
    }
  NRF_CE_LOW ();

  nrf_write_reg (R_STATUS,
		 R_CONFIG_MASK_RX_DR | R_CONFIG_MASK_TX_DS |
		 R_CONFIG_MASK_MAX_RT);

  ret = nrf_cmd_status (C_NOP);


}


struct frame
{
  uint32_t mid;
  uint32_t fragmentdata[WORDS_PER_FRAGMENT];
  unsigned short metad;
  unsigned short crc16;
};




#define size 32

void
test1 (void)
{
  struct frame dummy_frame;

  uint8_t sendBuf[32];
  dummy_frame.mid = 0;

  while (1)
    {
      UART2PutStr ("SEND\r\n");

      memcpy (sendBuf, &dummy_frame, 32);

      uint16_t crc = crc16 (sendBuf, size - 2);
      sendBuf[size - 2] = (crc >> 8) & 0xff;
      sendBuf[size - 1] = crc & 0xff;

      nrf_send_frame (sendBuf);

      dummy_frame.mid++;
      if (dummy_frame.mid % 10)
	{
	  UART2PutStr ("delay\r\n");
	  delay_ms (100);
	}
      delay_ms (100);

    }
}

void
nrf_config_set_x (nrfconfig config)
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



////////////


int
main (void)
{
  char outBuf[32];
  int c;
  struct NRF_CFG config;
  uint16_t cnt;
  uint8_t buf[32];
  int i;


  portsetup ();


  /* Configure PB frequency and wait states */
  SYSTEMConfigPerformance (SystemClock ());
//  UART1Init (SystemClock ());
  UART2Init (SystemClock ());

  INTConfigureSystem (INT_SYSTEM_CONFIG_MULT_VECTOR);
  INTEnableInterrupts ();

  UART2PutStr
    (".............................................................................hallo\r\n");
  UART2PutStr ("UART2 Welt\r\n");

  init_printf ();
  // try to reset nrf chip

  UART2PutStr ("nrf_init(),");

  SPI2_init ();

  NRF_POWER_OFF ();
  NRF_CS_HIGH ();
  NRF_CE_LOW ();
  delay_ms (10);
  NRF_POWER_ON ();
  delay_ms (100);
  nrf_write_reg (R_CONFIG,
		 R_CONFIG_PRIM_RX | R_CONFIG_PWR_UP | R_CONFIG_EN_CRC);
  nrf_write_reg (R_EN_AA, 0);
  nrf_write_reg (R_RF_SETUP, R_RF_SETUP_DR_2M | R_RF_SETUP_RF_PWR_3);
  nrf_write_reg (R_STATUS, R_STATUS_MAX_RT);


  UART2PutStr ("done\n\r");

  UART2PutStr ("nrf_config_set(),");
  config.nrmacs = 1;
  config.maclen[0] = 32;
  config.channel = 81;
  ultoa (outBuf, config.channel, 10);
  UART2PutStr ("channel: ,");
  UART2PutStr (outBuf);
  memcpy (config.mac0, "\x01\x02\x03\x02\x01", 5);
  memcpy (config.txmac, "\x1\x2\x3\x2\x1", 5);

  nrf_write_reg (R_SETUP_AW, R_SETUP_AW_5);
  nrf_write_reg (R_RF_CH, config.channel);


  for (i = 0; i < config.nrmacs; i++)
    {
      nrf_write_reg (R_RX_PW_P0 + i, config.maclen[i]);
      if (i == 0)
	{
	  nrf_write_reg_long (R_RX_ADDR_P0, 5, config.mac0);
	}
      else if (i == 1)
	{
	  nrf_write_reg_long (R_RX_ADDR_P1, 5, config.mac1);
	}
      else if (i > 1)
	{
	  nrf_write_reg_long (R_RX_ADDR_P0 + i, 1, config.mac2345 + i - 2);
	};
    };
  nrf_write_reg_long (R_TX_ADDR, 5, config.txmac);
  nrf_write_reg (R_EN_RXADDR, (1 << config.nrmacs) - 1);

//  nrf_config_set_x (&config);
//  nrf_config_set (&config);


  UART2PutStr ("done\n\r");


  UART2PutStr ("call\n\r");
  test1 ();








  while (1);

  buf[0] = 32;

  for (c = 1; c < 32; c++)
    buf[c] = c;

  UART2PutStr ("send:\n\r");
  cnt = 0;


  do
    {
      cnt++;
      buf[2] = cnt >> 8;
      buf[3] = cnt & 0xff;

      ultoa (outBuf, cnt, 10);
      UART2PutStr ("cnt: ");
      UART2PutStr (outBuf);
      UART2PutStr ("\n\r");

      uint16_t crc = crc16 (buf, 32 - 2);
      buf[32 - 2] = (crc >> 8) & 0xff;
      buf[32 - 1] = crc & 0xff;
      nrf_snd_pkt (32, buf);

      delay_ms (1000);
    }
  while (1);
  return 0;
}
