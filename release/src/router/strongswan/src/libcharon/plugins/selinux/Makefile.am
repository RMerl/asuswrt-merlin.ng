AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-selinux.la
else
plugin_LTLIBRARIES = libstrongswan-selinux.la
endif

libstrongswan_selinux_la_SOURCES = \
	selinux_plugin.h selinux_plugin.c \
	selinux_listener.h selinux_listener.c

libstrongswan_selinux_la_LDFLAGS = -module -avoid-version
