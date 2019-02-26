AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-constraints.la
else
plugin_LTLIBRARIES = libstrongswan-constraints.la
endif

libstrongswan_constraints_la_SOURCES = \
	constraints_plugin.h constraints_plugin.c \
	constraints_validator.h constraints_validator.c

libstrongswan_constraints_la_LDFLAGS = -module -avoid-version
