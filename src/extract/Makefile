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
	spikeparms2 \
	adextract 
	# crextract \
	# editheader \
	# esextract \
	# findspike \
	# header \
	# parmextract \
	# posextract \
	# showcmd \
	# spikeanal \
	# spikeavg \
	# textract

all: $(ALL)
	mv $(ALL) ../../build/extract/
.c:
	echo ".c"
	$(CC) $(CFLAGS) $< -o $@ $(INCLUDE) $(LIBS)

.c.o:
	echo ".c.o"
	$(CC) $(CFLAGS) $< -c $(INCLUDE) $(LIBS)

clean:
	rm -f *.o 
