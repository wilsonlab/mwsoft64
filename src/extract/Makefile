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
	adextract \
	crextract \
	editheader \
	esextract \
	findspike \
	parmextract \
	posextract \
	showcmd \
	spikeanal \
	spikeavg \
	textract \

all: $(ALL)

.c:
	$(CC) $(CFLAGS) $< -o $@ $(INCLUDE) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $< -c $(INCLUDE) $(LIBS)

clean:
	@rm -f *.o  
	@rm -rf *.dSYM
