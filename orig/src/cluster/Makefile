### MWL Makefile includes

# needed by include files
TOPDIR = ../../

# include commonly used definitions (CC,INCLUDE, LIBS etc...) and
# cflags (Date, RPM name ...)
include $(TOPDIR)definitions.mk

# uncomment next line for debug symbols
CFLAGS += -g3

# uncomment next line for max compiler optimization
CFLAGS += -O3


###

CFLAGS += -Wall


ALL = 	\
	adextract \
	crextract \
	editheader \
	esextract \
	findspike \
	header \
	parmextract \
	posextract \
	showcmd \
	spikeanal \
	spikeavg \
	spikeparms2 \
	textract

all: $(ALL)

.c:
	$(CC) $(CFLAGS) $< -o $@ $(INCLUDE) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $< -c $(INCLUDE) $(LIBS)

iolib.o: iolib.c 
	$(CC) $(CFLAGS) $< -c $(INCLUDE) 

iolib.a: iolib.o
	ar r iolib.a iolib.o
	ranlib iolib.a

clean:
	rm -f *.o $(ALL) iolib.a
