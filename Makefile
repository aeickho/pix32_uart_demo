# IMPORTANT: file 'procdefs.ld' absolutely must Must MUST be present in the
# working directory when compiling code for the UBW32!  Failure to do so will
# almost certainly result in your UBW32 getting 'bricked' and requiring
# re-flashing the bootloader (which, if you don't have a PICkit 2 or similar
# PIC programmer, you're screwed).  YOU HAVE BEEN WARNED.

# Type 'make' to build example program.  Put UBW32 device in Bootloader mode
# (PRG + RESET buttons), then type 'make write' to download program to the
# device.  Note: the latter will OVERWRITE any program you currently have
# installed there, including the Bit Whacker firmware, so be sure to keep
# around a copy of the original, which can be downloaded from the product
# web page here: http://www.schmalzhaus.com/UBW32/

HEX     = main.hex
FLASH   = sudo /home/tuxx/mips32/mphidflash -n -r
PROC    = 32MX220F032D
CC      = xc32-gcc
BIN2HEX = xc32-bin2hex
CFLAGS  = -g -Os -mips16e -mprocessor=$(PROC) -Werror -Wall -Wl,--report-mem,--defsym,_min_heap_size=0x1000 -I../flausch-projects/flauschlib

all: $(HEX)

$(HEX): main.elf
	$(BIN2HEX) -a $<

p_queue.o: p_queue.c p_queue.h
	$(CC) $(CFLAGS) -c $< -o $@

uart.o: uart.c uart.h
	$(CC) $(CFLAGS) -c $< -o $@

console.o: console.c uart.h console.h
	$(CC) $(CFLAGS) -c $< -o $@

printf.o: printf.c uart.h
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c uart.h console.h
	$(CC) $(CFLAGS) -c $< -o $@

%.elf: main.o console.o p_queue.o uart.o printf.o
	$(CC) $(CFLAGS) printf.o uart.o main.o p_queue.o console.o \
		-o $@ -lmchp_peripheral_$(PROC) -lm -lc ../flausch-projects/flauschlib/libflausch.a

write:
	$(FLASH) -w $(HEX)

clean:
	rm -f *.o *.elf *.hex
