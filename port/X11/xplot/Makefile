#  Xplot : the program : written by Matt Wilson 
#  Copyright 1990-1994

### MWL Makefile includes

# needed by include files
TOPDIR = ../../

# include commonly used definitions (CC, INCLUDE, LIBS etc...) and cflags
# (Date, RPM name ...)
include $(TOPDIR)definitions.mk

# uncomment next line for debug symbols
CFLAGS += -g3

# uncomment next line for max compiler optimization
CFLAGS += -O3


###


OBJ =	\
	main.o \
	arg.o \
	axis.o \
	buttons.o \
	color.o \
	commands.o \
	event_loop.o \
	graph.o \
	init.o \
	keys.o \
	label.o \
	load.o \
	menu.o \
	primitives.o \
	save.o \
	scale.o \
	text.o \
	tick.o \
	psdriver.o 

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) $< -c -o $@

default:	xplot	

xplot:		$(OBJ)
		$(CC) $(CFLAGS) $(INCLUDE) $(OBJ) -o $@ $(LIBS)

clean:
		rm -f *.o xplot

