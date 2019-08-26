CFLAGS?=-Wall -g -D_GNU_SOURCE -DDEBUG -Wstrict-prototypes -Wdeclaration-after-statement
CC = gcc

LIBS = -lnftnl -lmnl

ARCH := $(shell uname -m | grep -q "x86_64" && echo 64)

all:	test_nfct_get testnftnlrdr testnftpinhole

clean:
	$(RM) *.o testnftnlcrdr testnftpinhole testnftnlrdr_peer \
		test_nfct_get testnftnlrdr 

testnftnlrdr:	nftnlrdr.o nftnlrdr_misc.o testnftnlrdr.o upnpglobalvars.o $(LIBS)

testnftpinhole:	nftpinhole.o nftnlrdr_misc.o testnftpinhole.o upnpglobalvars.o $(LIBS)

test_nfct_get:	test_nfct_get.o test_nfct_get.o -lmnl -lnetfilter_conntrack

test_nfct_get.o:	test_nfct_get.c

testnftnlrdr_peer.o:	testnftnlrdr_peer.c

testnftnlrdr_dscp.o:	testnftnlrdr_dscp.c

nftnlrdr.o:		nftnlrdr.c nftnlrdr.h 

nftnlrdr_misc.o:	nftnlrdr_misc.c

nftpinhole.o:		nftpinhole.c nftpinhole.h

upnpglobalvars.o:	../upnpglobalvars.c ../upnpglobalvars.h
	$(CC) -c -o $@ $<
