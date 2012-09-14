#  Xview : the program : written by Matt Wilson 

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

CFLAGS += -DORIENT

OBJ =	\
	main.o \
	arg.o \
	box.o \
	color.o \
	command.o \
	contour.o \
	display.o \
	event_handler.o \
	keys.o \
	legend.o \
	load.o \
	init.o \
	event_loop.o \
	psdriver.o \
	primitives.o \
	scale.o \
	signal.o \
	surface.o \
	transform.o

HEADERS = \
	xview.h \
	xview_ext.h \
	xview_struct.h \
	xview_defs.h

PROGS =		xview

all:		xview	

.c.o:
	$(CC) $(CFLAGS) $< -c -o $@ $(INCLUDE)


arg.o: xview_defs.h

contour.o: $(HEADERS)

display.o: $(HEADERS)

xview:		$(OBJ)
		$(CC) $(CFLAGS) $(OBJ) -o $@ $(LIBS) $(INCLUDE)

clean:
		rm -f ${OBJ} ${PROGS}

