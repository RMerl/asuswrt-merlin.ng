## Process this file with automake to produce Makefile.in

# These binaries go in /sbin (not /usr/sbin), and that cannot be
# overridden at config time. The kernel "knows" the /sbin name.
sbindir = /sbin

man8_MANS	= nfsdcltrack.man
EXTRA_DIST	= $(man8_MANS)

AM_CFLAGS	+= -D_LARGEFILE64_SOURCE
sbin_PROGRAMS	= nfsdcltrack

noinst_HEADERS	= sqlite.h

nfsdcltrack_SOURCES = nfsdcltrack.c sqlite.c
nfsdcltrack_LDADD = ../../support/nfs/libnfs.a $(LIBSQLITE) $(LIBCAP)

MAINTAINERCLEANFILES = Makefile.in

