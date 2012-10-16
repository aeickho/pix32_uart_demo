RM = rm -f
CC = gcc
CFLAGS = -Wall -MMD 
CFLAGS =

RX_HOST_BIN  = rx_host
RX_HOST_OBJS = base128.o crc.o main.o


all: $(RX_HOST_BIN)

clean:
	$(RM) $(RX_HOST_BIN) $(RX_HOST_OBJS) *.d
        
$(RX_HOST_BIN): $(RX_HOST_OBJS)
	$(CC) -o $@ $^ $(LFLAGS)
                
include $(wildcard *.d)
                
.PHONY: all clean
                
                