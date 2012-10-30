
CC = gcc
INCLUDE =	-I$(TOPDIR)src -I/usr/X11R6/include
LIBS = 		-L/usr/X11R6/lib $(TOPDIR)/build/lib/iolib.a -lX11 -lm

# pass in compile time/date as DATE
DATE = `/bin/date`
CFLAGS  += -DDATE="\"$(DATE)\""
CFLAGS	+= -g