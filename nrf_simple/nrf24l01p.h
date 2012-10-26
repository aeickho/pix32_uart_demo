#ifndef NRF24L01P_H
#define NRF24L01P_H
#include <stdint.h>

#define MAX_PKT (32) // space for crc is supplied by the caller

// SPI commands
#define C_R_REGISTER		0x00
#define C_W_REGISTER		0x20
#define C_R_RX_PAYLOAD		0x61
#define C_W_TX_PAYLOAD		0xA0
#define C_FLUSH_TX		0xE1
#define C_FLUSH_RX		0xE2
#define C_REUSE_TX_PL		0xE3
#define C_R_RX_PL_WID		0x60
#define C_W_ACK_PAYLOAD		0xA8
#define C_W_TX_PAYLOAD_NOCACK	0xB0
#define C_NOP			0xFF

// Registers
#define R_CONFIG		0x00
#define R_EN_AA			0x01
#define R_EN_RXADDR		0x02
#define R_SETUP_AW		0x03
#define R_SETUP_RETR		0x04
#define R_RF_CH			0x05
#define R_RF_SETUP		0x06
#define R_STATUS		0x07
#define R_OBSERVE_TX		0x08
#define R_RPD			0x09
#define R_RX_ADDR_P0		0x0A
 #define R_RX_ADDR_P0_LEN 5
#define R_RX_ADDR_P1		0x0B
 #define R_RX_ADDR_P1_LEN 5
#define R_RX_ADDR_P2		0x0C
 #define R_RX_ADDR_P2_LEN 1
#define R_RX_ADDR_P3		0x0D
 #define R_RX_ADDR_P3_LEN 1
#define R_RX_ADDR_P4		0x0E
 #define R_RX_ADDR_P4_LEN 1
#define R_RX_ADDR_P5		0x0F
 #define R_RX_ADDR_P5_LEN 1
#define R_TX_ADDR		0x10
 #define R_TX_ADDR_LEN 5
#define R_RX_PW_P0		0x11
#define R_RX_PW_P1		0x12
#define R_RX_PW_P2		0x13
#define R_RX_PW_P3		0x14
#define R_RX_PW_P4		0x15
#define R_RX_PW_P5		0x16
#define R_FIFO_STATUS		0x17
#define R_DYNPD			0x1c

// Register Flags


// FIFO_STATUS reguister defintions
#define R_FIFO_STATUS_RESERVED2 0x80
#define R_FIFO_STATUS_TX_REUSE 	0x40
#define R_FIFO_STATUS_TX_FULL  	0x20
#define R_FIFO_STATUS_TX_EMPTY  0x10
#define R_FIFO_STATUS_RESERVED1 0x08
#define R_FIFO_STATUS_RESERVED0 0x04
#define R_FIFO_STATUS_FULL	0x02
#define R_FIFO_STATUS_RX_EMPTY	0x01

//CONFIG register definitions
#define R_CONFIG_RESERVED        0x80
#define R_CONFIG_MASK_RX_DR      0x40
#define R_CONFIG_MASK_TX_DS      0x20
#define R_CONFIG_MASK_MAX_RT     0x10
#define R_CONFIG_EN_CRC          0x08
#define R_CONFIG_CRCO            0x04
#define R_CONFIG_PWR_UP          0x02
#define R_CONFIG_PRIM_RX         0x01

//EN_AA register definitions
#define R_EN_AA_ENAA_P5          0x20
#define R_EN_AA_ENAA_P4          0x10
#define R_EN_AA_ENAA_P3          0x08
#define R_EN_AA_ENAA_P2          0x04
#define R_EN_AA_ENAA_P1          0x02
#define R_EN_AA_ENAA_P0          0x01
#define R_EN_AA_ENAA_NONE        0x00

//EN_RXADDR register definitions
#define R_EN_RXADDR_ERX_P5       0x20
#define R_EN_RXADDR_ERX_P4       0x10
#define R_EN_RXADDR_ERX_P3       0x08
#define R_EN_RXADDR_ERX_P2       0x04
#define R_EN_RXADDR_ERX_P1       0x02
#define R_EN_RXADDR_ERX_P0       0x01
#define R_EN_RXADDR_ERX_NONE     0x00

// RF_CH register definitions
#define R_RF_CH_BITS             0x7f

//RF_SETUP register definitions
#define R_RF_SETUP_CONT_WAVE     0x80
#define R_RF_SETUP_RF_DR_LOW     0x20
#define R_RF_SETUP_PLL_LOCK      0x10
#define R_RF_SETUP_RF_DR_HIGH    0x08
#define R_RF_SETUP_RF_PWR_0      0x00
#define R_RF_SETUP_RF_PWR_1      0x02
#define R_RF_SETUP_RF_PWR_2      0x04
#define R_RF_SETUP_RF_PWR_3      0x06

#define R_RF_SETUP_DR_1M         0x00
#define R_RF_SETUP_DR_2M         0x08
#define R_RF_SETUP_DR_250K       0x20

//SETUP_AW register definitions
#define R_SETUP_AW_3             0x01
#define R_SETUP_AW_4             0x02
#define R_SETUP_AW_5             0x03

//STATUS register definitions
#define R_STATUS_RX_DR           0x40
#define R_STATUS_TX_DS           0x20
#define R_STATUS_MAX_RT          0x10
#define R_STATUS_RX_P_NO         0x0E
#define R_STATUS_GET_RX_P_NO(x)  ((x&R_STATUS_RX_P_NO)>>1)
#define R_STATUS_RX_FIFO_EMPTY   0x0E
#define R_STATUS_TX_FULL         0x01

/* config structure */

struct NRF_CFG {
    uint8_t channel;
    uint8_t txmac[5];
    uint8_t mac0[5];
    uint8_t mac1[5];
    uint8_t mac2345[4];
    uint8_t nrmacs;
    uint8_t maclen[5];
};

typedef struct NRF_CFG * nrfconfig;

#endif

