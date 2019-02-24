AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libsimaka

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-simaka-pseudonym.la
else
plugin_LTLIBRARIES = libstrongswan-eap-simaka-pseudonym.la
libstrongswan_eap_simaka_pseudonym_la_LIBADD = $(top_builddir)/src/libsimaka/libsimaka.la
endif

libstrongswan_eap_simaka_pseudonym_la_SOURCES = \
	eap_simaka_pseudonym_plugin.h eap_simaka_pseudonym_plugin.c \
	eap_simaka_pseudonym_card.h eap_simaka_pseudonym_card.c \
	eap_simaka_pseudonym_provider.h eap_simaka_pseudonym_provider.c

libstrongswan_eap_simaka_pseudonym_la_LDFLAGS = -module -avoid-version
