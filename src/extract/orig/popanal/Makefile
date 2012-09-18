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

OBJS = 	\
	pop_args.o \
	pop_bayes.o \
	pop_corr.o \
	pop_func.o \
	pop_io.o \
	pop_main.o \
	pop_range.o \
	pop_replay.o \
	pop_reconstruct.o \
	pop_spiketrain.o \
	pop_tree.o 

ALL = popanal

default: popanal

.c:
	$(CC) $(CFLAGS) $< -o $@ $(INCLUDE) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $< -c $(INCLUDE) 

popanal: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(INCLUDE) $(LIBS)

pop_args.o: pop_defs.h

clean:
	rm -f *.o $(ALL)
