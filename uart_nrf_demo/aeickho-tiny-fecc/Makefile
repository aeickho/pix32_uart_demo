RM = rm -f
CC = gcc
CFLAGS = -Wall -Os -ansi -pedantic -MMD -march=native -mtune=native
LFLAGS =

DEMO2_BIN  = tfec3_demo2.exe
DEMO2_OBJS = tfec3_demo2.o tfec3.o crc.o

all: $(DEMO2_BIN)

clean:
	$(RM) $(DEMO2_BIN) $(DEMO2_OBJS) *.d

$(DEMO2_BIN): $(DEMO2_OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

include $(wildcard *.d)

.PHONY: all clean
