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
FLASH   = mphidflash -n -r
PROC    = 32MX220F032D
CC      = xc32-gcc
BIN2HEX = xc32-bin2hex
CFLAGS  = -g -Os -mips16e -mprocessor=$(PROC) -Wall -Wl,--report-mem,--defsym,_min_heap_size=0x1000

all: $(HEX)

$(HEX): main.elf
	$(BIN2HEX) -a $<

%.elf: %.c
	$(CC) $(CFLAGS) $< -o $@ -lmchp_peripheral_$(PROC) -lm -lc

write:
	$(FLASH) -w $(HEX)

clean:
	rm -f *.o *.elf *.hex
