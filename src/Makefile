### MWL Makefile includes

# needed by include files
TOPDIR = ../

# include commonly used definitions (CC,INCLUDE, LIBS etc...) and
# cflags (Date, RPM name ...)
include $(TOPDIR)definitions.mk

# uncomment next line for debug symbols
CFLAGS += -g3

# uncomment next line for max compiler optimization
CFLAGS += -O3

###

CFLAGS += -Wall

iolib.o: iolib.c 
	$(CC) $(CFLAGS) $< -c $(INCLUDE) 

iolib.a: iolib.o
	ar r iolib.a iolib.o
	ranlib iolib.a

clean:
	rm -f *.o iolib.a
