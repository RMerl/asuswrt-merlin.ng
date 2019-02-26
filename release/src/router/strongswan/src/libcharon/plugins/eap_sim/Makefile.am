AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libsimaka

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-sim.la
else
plugin_LTLIBRARIES = libstrongswan-eap-sim.la
libstrongswan_eap_sim_la_LIBADD = $(top_builddir)/src/libsimaka/libsimaka.la
endif

libstrongswan_eap_sim_la_SOURCES = \
	eap_sim_plugin.h eap_sim_plugin.c \
	eap_sim_peer.h eap_sim_peer.c \
	eap_sim_server.h eap_sim_server.c

libstrongswan_eap_sim_la_LDFLAGS = -module -avoid-version
