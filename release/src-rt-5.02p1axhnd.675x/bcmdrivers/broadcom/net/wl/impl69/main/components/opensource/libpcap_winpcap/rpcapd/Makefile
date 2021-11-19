################################
# Makefile for the remote daemon
################################

CC      = gcc
CFLAGS  = -pthread -DHAVE_REMOTE -DHAVE_SNPRINTF -DNDEBUG
#flags for debugging: -D_DEBUG -g -Wall

INCLUDE = -I../

LIB	= -lpcap -lcrypt
#Solaris: add '-lsocket'

LIBPATH	= -L../

# Files that are needed to compile this project
FILES = rpcapd.o daemon.o utils.o fileconf.o pcap-remote.o sockutils.o pcap-new.o

# Makefile syntax:
# Project name : dependencies
#  command line

rpcapd: $(FILES)
	$(CC) $(CFLAGS) $(INCLUDE) -o rpcapd $(FILES) $(LIBPATH) $(LIB)

rpcapd.o: rpcapd.c
	$(CC) $(CFLAGS) $(INCLUDE) -c rpcapd.c

daemon.o: daemon.c
	$(CC) $(CFLAGS) $(INCLUDE) -c daemon.c

utils.o: utils.c
	$(CC) $(CFLAGS) $(INCLUDE) -c utils.c

fileconf.o: fileconf.c
	$(CC) $(CFLAGS) $(INCLUDE) -c fileconf.c

sockutils.o: ../sockutils.c
	$(CC) $(CFLAGS) $(INCLUDE) -c ../sockutils.c

pcap-remote.o: ../pcap-remote.c
	$(CC) $(CFLAGS) $(INCLUDE) -c ../pcap-remote.c

pcap-new.o: ../pcap-new.c
	$(CC) $(CFLAGS) $(INCLUDE) -c ../pcap-new.c

clean:
	rm -f *.o
	rm rpcapd
