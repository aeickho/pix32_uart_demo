include ../Makefile.rules

#OPTIMIZE =  -O3

OBJECTS = tfec3.o   

include $(wildcard *.d)

all: $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(APPLIB_OBJECTS) $(APPLIB)
	rm -f *.o *.elf *.hex
	rm -f *.d


                         