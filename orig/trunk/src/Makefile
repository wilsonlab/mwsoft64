
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

CFLAGS += -Wall -W

#COMPILECUR = cc $@.c $(CFLAGS) -lcurses -ltermcap  -o $@ $(LIBS)
COMPILE = $(CC) $@.c $(CFLAGS) -o $@ $(LIBS) $(INCLUDE)

ALL = \
	atob \
	carve \
	csi \
	y 

.c:
	$(CC) $(CFLAGS) $@.c -o $@ $(INCLUDE) $(LIBS)
.c.o:
	$(COMPILE)

default: all

all: $(ALL)

clean: 
	rm -rf *.o $(ALL)
