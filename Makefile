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


# MCU to compile for:
PROC    = 32MX220F032D

# tool configuration
FLASH   = mphidflash -n -r
CC      = xc32-gcc
BIN2HEX = xc32-bin2hex
AR      = xc32-ar

# compiler config
OPTIMIZE ?= -Os -mips16e
#CFLAGS  = -g $(OPTIMIZE) -mprocessor=$(PROC) "-I$(APPLIB_PATH)/Include" -I.
CFLAGS  = -g $(OPTIMIZE) -mprocessor=$(PROC)  -I.
LDFLAGS = -g $(OPTIMIZE) -mprocessor=$(PROC) -Wl,--report-mem

APPLIB = 
APPLIB_OBJECTS = 

MAIN    = main
OBJECTS = main.o spi.o Pinguino.o 
                
                
#LIBS = -l mchp_peripheral_$(PROC)

OBJECTS =	main.o \
		spi.o \
		crc.o \
		byteorder.o \
		Pinguino.o \
                nrf24l01p.o \
		openbeacon.o
		

#############################################################################
# you can probably leave the following as they are:

all: $(MAIN).hex

$(MAIN).hex: $(MAIN).elf
	$(BIN2HEX) -a $<

$(MAIN).elf: $(OBJECTS) $(APPLIB) procdefs.ld
	$(CC) $(LDFLAGS) $(OBJECTS) $(APPLIB) $(LIBS) -o $(MAIN).elf

$(APPLIB): $(APPLIB_OBJECTS)
	$(AR) -r "$@" $(APPLIB_OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c "$<" -o "$@"

write: $(MAIN).hex
	$(FLASH) -w $(MAIN).hex

clean:
	- rm $(OBJECTS) $(APPLIB_OBJECTS) $(APPLIB)
	- rm -f *.o *.elf *.hex
