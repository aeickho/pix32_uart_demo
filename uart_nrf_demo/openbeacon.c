#include <stdint.h>
#include <string.h>
#include "openbeacon.h"
#include "nrf24l01p.h"
#include "byteorder.h"
#include "Pinguino.h"

#define SAVE_OPENBEACON 1
const uint8_t mac[5] = {1,2,3,2,1};

volatile uint32_t oid = 0;
volatile uint32_t seq = 0;
volatile uint8_t strength = 0;

char nickname[17];

                
static struct NRF_CFG oldconfig;

#define NICKNAME "PIC32_001000"
#define OID	0xdeadbeef

void openbeaconSetup(void)
{
    oid = OID;
    strength = 0;
    strcpy(nickname, NICKNAME);
}

static void openbeaconSendPacket(uint32_t id, uint32_t seq,
        uint8_t flags, uint8_t strength)
{
    uint8_t buf[16];
    uint8_t proto = 0x17;   //Tracking

    volatile uint16_t i;
    i = (20&0xfff)+1;
    while(i--);

    static uint32_t n = 123;
    if( --n == 0 ){
        n = 123;
        proto = 0x23;       //Nick name
    }

 proto = 0x23; 

    buf[0]=0x10; // Length: 16 bytes
    buf[1]=proto;
    if( proto == 0x17 ){
        buf[2]=flags;
        buf[3]=strength*85; // Send intensity

        uint32touint8p(seq, buf+4);
        uint32touint8p(id, buf+8);

        buf[12]=0xff; // salt (0xffff always?)
        buf[13]=0xff;
        nrf_snd_pkt_crc_encr(16,buf,NULL);
    }else{
        if( strlen(nickname) > 8 )
            buf[1] = 0x24;
        nrf_set_strength(3);
        uint32touint8p(id, buf+2);
        memcpy(buf+6, nickname, 8);
        nrf_snd_pkt_crc_encr(16, buf,NULL);
        if( strlen(nickname) < 9 )
            return;
        buf[1]=0x25;
        memcpy(buf+6, nickname+8, 8);
        _delay_ms(100);
        nrf_snd_pkt_crc_encr(16, buf,NULL);
    }   
}

void openbeaconSend(void)
{
    openbeaconSetup();
    nrf_config_get(&oldconfig);

    nrf_set_channel(OPENBEACON_CHANNEL);
    nrf_set_strength(strength);
    nrf_set_tx_mac(sizeof(mac), mac);

    openbeaconSendPacket(oid, seq++, 0xFF, strength++);
    if( strength == 4 )
        strength = 0;
    nrf_config_set(&oldconfig);
    nrf_set_strength(3);
}

