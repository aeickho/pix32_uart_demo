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

include Makefile.rules

MAIN    = main


DIRS = aeickho-tiny-fecc tools

OBJECTS =	main.o \
		portsetup.o \
		uart2.o \
		myspi2.o \
		crc.o \
		byteorder.o \
		Pinguino.o \
                nrf24l01p.o \
		general_exception_handler.o \
		tfec3_tools.o
#		uart1.o
#		myspi1.o  \
		

#############################################################################
# you can probably leave the following as they are:

all: $(MAIN).hex

$(MAIN).hex: $(MAIN).elf
	$(BIN2HEX) -a $<

$(MAIN).elf: $(OBJECTS) $(APPLIB) procdefs.ld Makefile
	for i in $(DIRS); do make -C $$i; done
#	$(CC) $(LDFLAGS) $(OBJECTS) $(APPLIB) $(LIBS) aeickho-tiny-fecc/sg_crc.o -o $(MAIN).elf
	$(CC) $(LDFLAGS) $(OBJECTS) $(APPLIB) $(LIBS) tools/printf.o aeickho-tiny-fecc/tfec3.o -o $(MAIN).elf
#	$(CC) $(LDFLAGS) $(OBJECTS) $(APPLIB) $(LIBS)  -o $(MAIN).elf

 

$(APPLIB): $(APPLIB_OBJECTS)
	$(AR) -r "$@" $(APPLIB_OBJECTS)

#%.o: %.c
#	$(CC) $(CFLAGS) -c "$<" -o "$@"

write: $(MAIN).hex
	$(FLASH) -w $(MAIN).hex

clean:
	- rm -f $(OBJECTS) $(APPLIB_OBJECTS) $(APPLIB)
	- rm -f *.o *.elf *.hex
	- rm -f *.d
	for i in $(DIRS); do make -C $$i clean; done	

ctags:
	$(CC) -g -Os  -mprocessor=$(PROC) -M -c $(MAIN).c  | sed -e 's/[\\ ]/\n/g' | sed -e '/^$$/d' -e '/\.o:[ \t]*$$/d'  | ctags -L - --c++-kinds=+p --fields=+iaS --extra=+q

include $(wildcard *.d)

