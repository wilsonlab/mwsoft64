### MWL Makefile includes

# needed by include files
TOPDIR = ../../../

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
	behav

OBJS = \
	behav_alloc.o \
	behav_ang.o \
	behav_corr.o \
	behav_bounds.o \
	behav_flip.o \
	behav_func.o \
	behav_main.o \
	behav_occ.o \
	behav_read.o \
	behav_stat.o \
	behav_write.o 


all: $(ALL)

.c:
	$(CC) $(CFLAGS) $< -o $@ $(INCLUDE) 

.c.o:
	$(CC) $(CFLAGS) $< -c $(INCLUDE)

obehav: obehav.o
	$(CC) $(CFLAGS) obehav.o -o $@ $(INCLUDE) $(LIBS)

behav: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(INCLUDE) $(LIBS)

clean:
	rm -f *.o $(ALL)
