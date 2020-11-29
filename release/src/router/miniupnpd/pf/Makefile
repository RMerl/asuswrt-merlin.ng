# $Id: Makefile,v 1.7 2020/05/21 00:10:56 nanard Exp $
# made for GNU Make (and BSD make)
CFLAGS = -Wall -g -DTEST
CFLAGS += -Wextra
CFLAGS += -I..
EXECUTABLES = testobsdrdr testpfpinhole

all:	$(EXECUTABLES)

clean:
	rm -f *.o $(EXECUTABLES)

testobsdrdr:	testobsdrdr.o obsdrdr.o getifaddr.o
	$(CC) $(CFLAGS) -o $@ $>

testpfpinhole:	testpfpinhole.o obsdrdr.o pfpinhole.o getifaddr.o
	$(CC) $(CFLAGS) -o $@ $>

obsdrdr.o:	obsdrdr.c obsdrdr.h

pfpinhole.o:	pfpinhole.c pfpinhole.h

testobsdrdr.o:	testobsdrdr.c obsdrdr.h

testpfpinhole.o:	testpfpinhole.c pfpinhole.h

getifaddr.o:	../getifaddr.c
