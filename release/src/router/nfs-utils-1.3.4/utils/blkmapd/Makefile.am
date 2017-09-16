## Process this file with automake to produce Makefile.in

man8_MANS	= blkmapd.man
EXTRA_DIST = $(man8_MANS)

AM_CFLAGS	+= -D_LARGEFILE64_SOURCE
sbin_PROGRAMS	= blkmapd

blkmapd_SOURCES = \
	device-discovery.c \
	device-inq.c \
	device-process.c \
	dm-device.c \
	device-discovery.h

blkmapd_LDADD = -ldevmapper ../../support/nfs/libnfs.a

MAINTAINERCLEANFILES = Makefile.in

