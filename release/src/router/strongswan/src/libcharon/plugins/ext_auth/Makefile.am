AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-ext-auth.la
else
plugin_LTLIBRARIES = libstrongswan-ext-auth.la
endif

libstrongswan_ext_auth_la_SOURCES = ext_auth_plugin.h ext_auth_plugin.c \
	ext_auth_listener.h ext_auth_listener.c

libstrongswan_ext_auth_la_LDFLAGS = -module -avoid-version
