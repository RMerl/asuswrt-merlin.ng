## Process this file with automake to produce Makefile.in

man8_MANS = nfsidmap.man
sbin_PROGRAMS	= nfsidmap

nfsidmap_SOURCES = nfsidmap.c
nfsidmap_LDADD = $(LIBNFSIDMAP) -lkeyutils ../../support/nfs/libnfs.a

MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = id_resolver.conf $(man8_MANS)
