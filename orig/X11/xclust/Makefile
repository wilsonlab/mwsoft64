#  Xclust : the program : written by Matt Wilson 

### MWL Makefile includes

# needed by include files
TOPDIR = ../../

# include commonly used definitions (CC, etc...) and cflags (Date, RPM name ...)
include $(TOPDIR)definitions.mk

# uncomment next line for debug symbols
CFLAGS += -g3

# uncomment next line for max compiler optimization
CFLAGS += -O3


###

#name of xclust executable to be built
XCLUST = xclust3

# defining DEBUG macro causes X events to be synchronized with errors
#CFLAGS  +=	 -DDEBUG



OBJ =	\
	main.o \
	arg.o \
	axis.o \
	buttons.o \
	clust_disp.o \
	clust_io.o \
	clust_main.o \
	clust_menu.o \
	color.o \
	commands.o \
	defaults.o \
	event_loop.o \
	graph.o \
	init.o \
	job.o \
	keys.o \
	label.o \
	load.o \
	menu.o \
	primitives.o \
	scale.o \
	text.o \
	tick.o \
	psdriver.o \

.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) $< -c -o $@

default:	$(XCLUST)	
xclust:	$(XCLUST)

arg.o: xclust_defs.h
event_loop.o: xclust_defs.h

$(XCLUST):		$(OBJ)
		$(CC) $(CFLAGS) $(OBJ) $(INCLUDE) -o $@ $(LIBS)

clean:
		rm -f *.o $(XCLUST)
