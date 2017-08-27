ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = $(man_MANS) include/linux

SUBDIRS = src utils include examples

man_MANS = #nfnetlink_queue.3 nfnetlink_queue.7

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnetfilter_queue.pc

EXTRA_DIST += Make_global.am
