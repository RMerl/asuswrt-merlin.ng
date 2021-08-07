## Process this file with automake to produce Makefile.in.

if ENABLE_BIN
bin_PROGRAMS = haveged
else
sbin_PROGRAMS = haveged
endif

AM_CFLAGS=-Wall -Wextra -I..

####nolibtool_start##
##haveged_SOURCES = haveged.c havege.c havegetune.c havegecollect.c havegetest.c havegecmd.c \
##                  cpuid-43.h haveged.h havege.h havegetune.h havegecollect.h havegetest.h oneiteration.h \
##                  havegecmd.h
##
##haveged_LDADD = @HA_LDFLAGS@
####nolibtool_end##
##libtool_start##
lib_LTLIBRARIES = libhavege.la
libhavege_la_CPPFLAGS =
libhavege_la_LDFLAGS = -version-number @HAVEGE_LT_VERSION@
libhavege_la_LIBADD = @HA_LDFLAGS@

libhavege_la_SOURCES = havege.c havegetune.c havegecollect.c havegetest.c \
            cpuid-43.h havege.h havegetune.h havegecollect.h havegetest.h oneiteration.h

pkginclude_HEADERS = havege.h

haveged_SOURCES = haveged.c haveged.h havegecmd.c havegecmd.h

haveged_LDADD = @HA_LDFLAGS@ libhavege.la
##libtool_end##

MAINTAINERCLEANFILES = Makefile.in
