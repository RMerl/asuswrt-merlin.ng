# -*- Makefile -*-

AM_CFLAGS        = ${regular_CFLAGS}
AM_CPPFLAGS      = ${regular_CPPFLAGS} -I${top_builddir}/include -I${top_srcdir}/include ${kinclude_CPPFLAGS}
AM_LDFLAGS       = ${regular_LDFLAGS}

pkgconfig_DATA      = libiptc.pc libip4tc.pc libip6tc.pc

lib_LTLIBRARIES     = libip4tc.la libip6tc.la
libip4tc_la_SOURCES = libip4tc.c
libip4tc_la_LDFLAGS = -version-info 2:0:0
libip6tc_la_SOURCES = libip6tc.c
libip6tc_la_LDFLAGS = -version-info 2:0:0

EXTRA_DIST = libiptc.c linux_list.h
