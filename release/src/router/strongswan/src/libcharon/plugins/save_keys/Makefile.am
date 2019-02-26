AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-save-keys.la
else
plugin_LTLIBRARIES = libstrongswan-save-keys.la
endif

libstrongswan_save_keys_la_SOURCES = \
	save_keys_plugin.h save_keys_plugin.c \
	save_keys_listener.c save_keys_listener.h

libstrongswan_save_keys_la_LDFLAGS = -module -avoid-version
