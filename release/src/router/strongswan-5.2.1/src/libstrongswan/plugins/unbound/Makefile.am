AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-DIPSEC_CONFDIR=\"${sysconfdir}\"

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-unbound.la
else
plugin_LTLIBRARIES = libstrongswan-unbound.la
endif

libstrongswan_unbound_la_SOURCES = \
	unbound_plugin.h unbound_plugin.c \
	unbound_resolver.c unbound_resolver.h \
	unbound_rr.h unbound_rr.c \
	unbound_response.h unbound_response.c

libstrongswan_unbound_la_LDFLAGS = -module -avoid-version
libstrongswan_unbound_la_LIBADD  = -lunbound -lldns
