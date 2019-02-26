AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-acert.la
else
plugin_LTLIBRARIES = libstrongswan-acert.la
endif

libstrongswan_acert_la_SOURCES = \
	acert_validator.h acert_validator.c \
	acert_plugin.h acert_plugin.c

libstrongswan_acert_la_LDFLAGS = -module -avoid-version
