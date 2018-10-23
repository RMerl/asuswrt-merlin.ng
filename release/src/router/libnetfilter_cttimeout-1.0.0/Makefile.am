include $(top_srcdir)/Make_global.am

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src include examples
DIST_SUBDIRS = src include examples

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnetfilter_cttimeout.pc

${pkgconfig_DATA}: ${top_builddir}/config.status
