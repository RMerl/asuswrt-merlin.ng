AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libsimaka

AM_CFLAGS = \
	${pcsclite_CFLAGS} \
	$(PLUGIN_CFLAGS)

libstrongswan_eap_sim_pcsc_la_LDFLAGS = -module -avoid-version
libstrongswan_eap_sim_pcsc_la_LIBADD  = ${pcsclite_LIBS}

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-sim-pcsc.la
else
plugin_LTLIBRARIES = libstrongswan-eap-sim-pcsc.la
libstrongswan_eap_sim_pcsc_la_LIBADD += $(top_builddir)/src/libsimaka/libsimaka.la
endif

libstrongswan_eap_sim_pcsc_la_SOURCES = \
	eap_sim_pcsc_plugin.h eap_sim_pcsc_plugin.c \
	eap_sim_pcsc_card.h eap_sim_pcsc_card.c
