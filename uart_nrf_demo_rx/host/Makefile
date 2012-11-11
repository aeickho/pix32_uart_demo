RM = rm -f
CC = gcc
CFLAGS = -Wall -MMD -std=c99 -g

RX_HOST_BIN  = rx_host
RX_HOST_OBJS = base128.o crc.o main.o rxmsg.o aeickho-tiny-fecc/tfec3.o 


all: $(RX_HOST_BIN)

clean:
	$(RM) $(RX_HOST_BIN) $(RX_HOST_OBJS) *.d
        
$(RX_HOST_BIN): $(RX_HOST_OBJS)
	$(CC) -o $@ $^ $(LFLAGS) $(CFLAGS)
                
include $(wildcard *.d)
                
.PHONY: all clean
                
                