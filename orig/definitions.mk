############################################
#commonly used definitions for mwl makefiles

CC = gcc
INCLUDE =	-I$(TOPDIR)include -I/usr/X11R6/include
LIBS = 		-L/usr/X11R6/lib $(TOPDIR)lib/iolib.a -lX11 -lm


############################################
# common cflag additions for mwl Makefiles

# pass in compile time/date as DATE
DATE = `/bin/date`
CFLAGS  += -DDATE="\"$(DATE)\""

# if we are building an RPM, then pass this string in for use in creating
# -version message
ifdef MWSOFT_RPM_STRING
CFLAGS += -DMWSOFT_RPM_STRING=$(MWSOFT_RPM_STRING)
endif
