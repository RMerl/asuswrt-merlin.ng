# -*- Makefile -*-

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = include lib man python tests

pkgconfig_DATA = libnl-3.0.pc \
		 libnl-route-3.0.pc \
		 libnl-genl-3.0.pc \
		 libnl-nf-3.0.pc

if ENABLE_CLI
SUBDIRS += src
pkgconfig_DATA += libnl-cli-3.0.pc
endif


pkgsysconfdir = ${sysconfdir}/libnl
pkgsysconf_DATA = etc/pktloc etc/classid

EXTRA_DIST = \
	$(pkgsysconf_DATA)
