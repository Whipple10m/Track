#
#
# Flags that might be overriden by top makefile
#

CC=cc
CCFLAG=-O -Wall
LDFLAG=-O -s 
SYSLIB=-lX11 -lm

include /usr/local/include/mkconfig.h

#
# Flags the compiler will see
#

CFLAGS= -O -Wall -I../FORMS $(XINCLUDE)
LDFLAGS = $(LDFLAG) 
LIBS=-L/usr/src/xforms/FORMS -lforms -ltooltips $(SYSLIB)
BINDIR = /home/track10/bin
DOCDIR = /home/track10/docs

#
#


all:	track 


track: clean track.o 
	-$(CC) $(LDFLAGS) -o $@ track.o $(LIBS)

track.o: track.h

clean:
	-rm -f  track *.o core a.out *~

empty: clean
	-rm -f $(DEMOS) track old*

install:;   -echo -n "Installing 10meter track program... "
	    @cp track $(BINDIR)/track
	    @chgrp users $(BINDIR)/track
	    @chown root $(BINDIR)/track
	    @chmod 6755 $(BINDIR)/track
	    -echo "done"	








