#include <sysdefs.h>
#include <sysinit.h>
#include <string.h>
#include "basic/basic.h"
#include "funk/mesh.h"
#include "funk/nrf24l01p.h"
#include "basic/byteorder.h"
#include "basic/random.h"
#include "basic/config.h"
#include "lcd/print.h"

char meshgen=0; // Generation
char meshincctr=0;
char meshmsg=0;
char meshnice=0;

char mesh_mode=0;
#define MM_TIME (1<<0)
#define MM_ENC  (1<<1)
MPKT meshbuffer[MESHBUFSIZE];

#include "SECRETS"

struct NRF_CFG oldconfig;

static int mesh_gt(char curgen, char newgen){
    unsigned char dif=curgen-newgen;
    if(curgen==0)
        return 1;
	if(newgen==0)
		return 0;
    return (dif>128);
};

void initMesh(void){
    for(int i=0;i<MESHBUFSIZE;i++){
        meshbuffer[i].flags=MF_FREE;
    };
    memset(meshbuffer[0].pkt,0,MESHPKTSIZE);
    meshbuffer[0].pkt[0]='T';
    MO_TIME_set(meshbuffer[0].pkt,getSeconds());
    meshbuffer[0].flags=MF_USED;
};

#define MP_OK     0
#define MP_SEND   1
#define MP_RECV   2
#define MP_IGNORE 4
int mesh_sanity(uint8_t * pkt){
    if(MO_TYPE(pkt)>0x7f || MO_TYPE(pkt)<0x20)
        return MP_SEND;
    if(MO_TYPE(pkt)=='T' && MO_BODY(pkt)[5])
           return MP_SEND;
    if(MO_TYPE(pkt)=='T' && MO_TIME(pkt)<86400)
           return MP_OK;
    if(MO_TYPE(pkt)>='A' && MO_TYPE(pkt)<='Z'){
        if(MO_TIME(pkt)>1370340000) /* 4.Jun 2013 */
            return MP_SEND;
        if(MO_TIME(pkt)<1325376000) /* 1.1.2012 */
            return MP_SEND;
    }else if(MO_TYPE(pkt)>='a' && MO_TYPE(pkt)<='z'){
        if(MO_TIME(pkt)>16777216) /* 3-byte only */
            return MP_SEND;
        if(MO_TIME(pkt)<0)
            return MP_SEND;
    };
    if(MO_TYPE(pkt)!='A' && 
       MO_TYPE(pkt)!='a' && 
       MO_TYPE(pkt)!='i' && 
       MO_TYPE(pkt)!='B' && 
       MO_TYPE(pkt)!='E' && 
       MO_TYPE(pkt)!='F' && 
       MO_TYPE(pkt)!='G' && 
       MO_TYPE(pkt)!='T'
            ){
        return MP_IGNORE;
    };
    return MP_OK;
};

MPKT * meshGetMessage(uint8_t type){
    int free=-1;
    for(int i=0;i<MESHBUFSIZE;i++){
        if ( ((meshbuffer[i].flags&MF_USED)==0) && free<0 )
            free=i;
        if ( (meshbuffer[i].flags&MF_USED) &&
                (MO_TYPE(meshbuffer[i].pkt) == type)){
            free=i;
            break;
        };
    };
    if(free==-1){ // Buffer full. Ah well. Kill a random packet
        free= (int)(getRandom() % MESHBUFSIZE);
        meshbuffer[free].flags=MF_FREE;
    };
    if(meshbuffer[free].flags==MF_FREE){
        memset(&meshbuffer[free],0,sizeof(MPKT));
        MO_TYPE_set(meshbuffer[free].pkt,type);
        MO_GEN_set(meshbuffer[free].pkt,meshgen);
        meshbuffer[free].flags=MF_USED;
    };
    return &meshbuffer[free];
};

void meshPanic(uint8_t * pkt,int bufno){
#if 0
    static int done=0;
    if(!done){
        gpioSetValue (RB_LED0, 1-gpioGetValue(RB_LED0));
	setSystemFont();
	lcdClear();
	lcdPrint("PANIC[");
	lcdPrint(IntToStrX(bufno,2));
	lcdPrint("]");
	lcdNl();
	for(int i=0;i<32;i++){
	    lcdPrint(IntToStrX(pkt[i],2));
	    if(i%6==2){
		lcdPrint(" ");
	    };
	    if(i%6==5){
		lcdNl();
	    };
	}
	lcdPrint(" ");
	lcdPrint(IntToStrX(crc16(pkt,30),4));
	lcdRefresh();
	while ((getInputRaw())==BTN_NONE);
    };
    done=1;
#endif
};

void mesh_cleanup(void){
    time_t now=getSeconds();
    for(int i=1;i<MESHBUFSIZE;i++){
        if(meshbuffer[i].flags&MF_LOCK)
            continue;
        if(meshbuffer[i].flags&MF_USED){
            if (MO_GEN(meshbuffer[i].pkt)<meshgen)
                meshbuffer[i].flags=MF_FREE;
            if (MO_TYPE(meshbuffer[i].pkt)>='a' &&
                    MO_TYPE(meshbuffer[i].pkt)<='z'){
                ;
            }else{
                if (MO_TIME(meshbuffer[i].pkt)<now)
                    meshbuffer[i].flags=MF_FREE;
                if (MO_TIME(meshbuffer[i].pkt)-now>SECS_DAY)
                    meshbuffer[i].flags=MF_FREE;
            };
            if((mesh_sanity(meshbuffer[i].pkt)&MP_SEND)!=0){
                meshbuffer[i].flags=MF_FREE;
                meshPanic(meshbuffer[i].pkt,i);
            };
        };
    };
};

void mesh_sendloop(void){
    int ctr=0;
    __attribute__ ((aligned (4))) uint8_t buf[32];
    int status;
    uint32_t rnd=0xffffffff;

    if(meshnice)
        rnd=getRandom();

    nrf_config_get(&oldconfig);
    nrf_set_channel(MESH_CHANNEL);
    nrf_set_tx_mac(strlen(MESH_MAC),(uint8_t*)MESH_MAC);

    // Update [T]ime packet
    MO_TIME_set(meshbuffer[0].pkt,getSeconds());
    MO_GEN_set(meshbuffer[0].pkt,meshgen);
    if(GLOBAL(privacy)==0){
        uint32touint8p(GetUUID32(),meshbuffer[0].pkt+26);
        uint32touint8p(getrelease(),meshbuffer[0].pkt+22);
    }else{
        uint32touint8p(0,meshbuffer[0].pkt+26);
        uint32touint8p(0,meshbuffer[0].pkt+22);
    };

    MO_BODY(meshbuffer[0].pkt)[4]=meshnice;

    for (int i=0;i<MESHBUFSIZE;i++){
        if(!meshbuffer[i].flags&MF_USED)
            continue;
        if(meshbuffer[i].flags&MF_LOCK)
            continue;
        if(meshnice&0xf){
            if((rnd++)%0xf < (meshnice&0x0f)){
                meshincctr++;
                continue;
            };
        };
        ctr++;
        memcpy(buf,meshbuffer[i].pkt,MESHPKTSIZE);
        status=nrf_snd_pkt_crc_encr(MESHPKTSIZE,buf,NULL);
        //Check status? But what would we do...
    };

    nrf_config_set(&oldconfig);
};

void mesh_recvqloop_setup(void){

    nrf_config_get(&oldconfig);

    nrf_set_channel(MESH_CHANNEL);
    nrf_set_rx_mac(0,MESHPKTSIZE,strlen(MESH_MAC),(uint8_t*)MESH_MAC);

    mesh_cleanup();

    nrf_rcv_pkt_start();
};

static inline uint32_t popcount(uint32_t *buf, uint8_t n){
    int cnt=0;
    do {
        unsigned m = *buf++;
        m = (m & 0x55555555) + ((m & 0xaaaaaaaa) >> 1);
        m = (m & 0x33333333) + ((m & 0xcccccccc) >> 2);
        m = (m & 0x0f0f0f0f) + ((m & 0xf0f0f0f0) >> 4);
        m = (m & 0x00ff00ff) + ((m & 0xff00ff00) >> 8);
        m = (m & 0x0000ffff) + ((m & 0xffff0000) >> 16);
        cnt += m;
    } while(--n);
    return cnt;
}

uint8_t mesh_recvqloop_work(void){
    __attribute__ ((aligned (4))) uint8_t buf[32];
    signed int len;

        len=nrf_rcv_pkt_poll_dec(sizeof(buf),buf,NULL);

        // Receive
        if(len<=0){
            return 0;
        };

        if(mesh_sanity(buf)){
            meshincctr++;
            if((mesh_sanity(buf)&MP_RECV)!=0){
                meshPanic(buf,-len);
            };
            return 0;
        };

        // New mesh generation?
        if(MO_TYPE(buf)=='T'){
            if(mesh_gt(meshgen,MO_GEN(buf))){
                _timet=0;
                meshincctr=0;
                meshnice=MO_BODY(buf)[4];
                meshgen=MO_GEN(buf);
				mesh_mode&=~MM_TIME;
            };
        };

        // Discard packets with wrong generation
        if(meshgen != MO_GEN(buf)){
            return 0;
        };

        // Set new time iff I don't have a valid one.
		if((mesh_mode & MM_TIME)==0){
			if(MO_TYPE(buf)=='T'){
				time_t toff=MO_TIME(buf)-((getTimer()+(600/SYSTICKSPEED))/(1000/SYSTICKSPEED));
				_timet = toff;
				if(meshgen==0 && MO_TIME(buf)<60*60*24){
					; // still not valid
				}else{
					mesh_mode|=MM_TIME; // Got a time now.
				};
				return 1;
			};
		};

        // Safety: Truncate ascii packets by 0-ing the CRC
        buf[MESHPKTSIZE-2]=0;

        // Store packet in a same/free slot
        MPKT* mpkt=meshGetMessage(MO_TYPE(buf));

        // Propagation test
        if(MO_TYPE(buf)=='B'){
            if(! mesh_gt(MO_BODY(mpkt->pkt)[0],MO_BODY(buf)[0]) )
                return 0;
            (*(
               (uint32_t*)(MO_BODY(buf)+6)
               ))++;
	    if(GLOBAL(privacy)==0)
		    uint32touint8p(GetUUID32(),MO_BODY(buf)+20);
            MO_TIME_set(mpkt->pkt,0);
        };
#if 0
        // Schnitzel
        if(MO_TYPE(buf)=='Z'){
            mpkt->flags=MF_USED|MF_LOCK;
            MO_TIME_set(mpkt->pkt,getSeconds());
            MO_GEN_set(mpkt->pkt,0x70);
            for(int i=6;i<MESHPKTSIZE;i++)
                mpkt->pkt[i]|=buf[i];

            int score=popcount((uint32_t*)MO_BODY(mpkt->pkt),6);

            MPKT* reply=meshGetMessage('z');

            if(MO_TIME(reply->pkt)>=score)
                return 1;

            MO_TIME_set(reply->pkt,score);
            strcpy((char*)MO_BODY(reply->pkt),GLOBAL(nickname));
            if(GLOBAL(privacy)==0){
                    uint32touint8p(GetUUID32(),meshbuffer[0].pkt+26);
                    meshbuffer[0].pkt[25]=0;
            };
            return 1;
        };
#endif

        // only accept newer/better packets
        if(mpkt->flags==MF_USED)
            if(MO_TIME(buf)<=MO_TIME(mpkt->pkt))
                return 2;

#if 0
        if((MO_TYPE(buf)>='A' && MO_TYPE(buf)<='C') ||
                (MO_TYPE(buf)>='a' && MO_TYPE(buf)<='c'))
                    meshmsg=1;
#endif

        memcpy(mpkt->pkt,buf,MESHPKTSIZE);
        mpkt->flags=MF_USED;

        return 1;
};

void mesh_recvqloop_end(void){
    nrf_rcv_pkt_end();
    nrf_config_set(&oldconfig);
}

void mesh_recvloop(void){ /* unused: replaced by mesh_recvloop_plus */
    int recvend=M_RECVTIM/SYSTICKSPEED+getTimer();
    int pktctr=0;

    mesh_recvqloop_setup();
    do{
        if( mesh_recvqloop_work() ){
            pktctr++;
        }else{
            delayms_power(10);
        };
    }while(getTimer()<recvend && pktctr<MESHBUFSIZE);
    mesh_recvqloop_end();
};

uint8_t mesh_recvloop_plus(uint8_t state){
    static int recvend=0;
    static int pktctr=0;

    if (state==0){
            recvend=M_RECVTIM/SYSTICKSPEED+getTimer();
            pktctr=0;

            mesh_recvqloop_setup();
            state=1;
    };
    if(state==1){
            if( mesh_recvqloop_work() ){
                pktctr++;
            }else{
                delayms_power(10);
            };
            if(getTimer()>recvend || pktctr>MESHBUFSIZE)
                state=QS_END;
    };
    return state;
};

void mesh_systick(void){
    static int rcvctr=0;
    static int sendctr=0;

    if(rcvctr--<0){
        push_queue_plus(&mesh_recvloop_plus);
        rcvctr=M_RECVINT/SYSTICKSPEED/2;
        rcvctr+=getRandom()%(rcvctr*2);
    };

    if(sendctr--<0){
        push_queue(&mesh_sendloop);
        sendctr=M_SENDINT/SYSTICKSPEED/2;
        sendctr+=getRandom()%(sendctr*2);
    };
};

