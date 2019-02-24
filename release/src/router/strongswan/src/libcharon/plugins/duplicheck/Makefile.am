AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-DIPSEC_PIDDIR=\"${piddir}\"

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-duplicheck.la
else
plugin_LTLIBRARIES = libstrongswan-duplicheck.la
endif

libstrongswan_duplicheck_la_SOURCES = duplicheck_plugin.h duplicheck_plugin.c \
	duplicheck_listener.h duplicheck_listener.c \
	duplicheck_notify.h duplicheck_notify.c \
	duplicheck_msg.h

libstrongswan_duplicheck_la_LDFLAGS = -module -avoid-version

ipsec_PROGRAMS = duplicheck
duplicheck_SOURCES = duplicheck.c
