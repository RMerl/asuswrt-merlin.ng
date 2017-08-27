AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-coupling.la
else
plugin_LTLIBRARIES = libstrongswan-coupling.la
endif

libstrongswan_coupling_la_SOURCES = coupling_plugin.h coupling_plugin.c \
	coupling_validator.h coupling_validator.c

libstrongswan_coupling_la_LDFLAGS = -module -avoid-version
