AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-gcm.la
else
plugin_LTLIBRARIES = libstrongswan-gcm.la
endif

libstrongswan_gcm_la_SOURCES = \
	gcm_plugin.h gcm_plugin.c \
	gcm_aead.h gcm_aead.c

libstrongswan_gcm_la_LDFLAGS = -module -avoid-version
