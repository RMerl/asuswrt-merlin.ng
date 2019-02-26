AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-revocation.la
else
plugin_LTLIBRARIES = libstrongswan-revocation.la
endif

libstrongswan_revocation_la_SOURCES = \
	revocation_plugin.h revocation_plugin.c \
	revocation_validator.h revocation_validator.c

libstrongswan_revocation_la_LDFLAGS = -module -avoid-version
