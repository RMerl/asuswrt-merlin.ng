AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-android-dns.la
else
plugin_LTLIBRARIES = libstrongswan-android-dns.la
endif

libstrongswan_android_dns_la_SOURCES = \
	android_dns_plugin.c android_dns_plugin.h \
	android_dns_handler.c android_dns_handler.h

libstrongswan_android_dns_la_LDFLAGS = -module -avoid-version
libstrongswan_android_dns_la_LIBADD  = -lcutils
