AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libradius

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-radius.la
else
libstrongswan_eap_radius_la_LIBADD = $(top_builddir)/src/libradius/libradius.la
plugin_LTLIBRARIES = libstrongswan-eap-radius.la
endif

libstrongswan_eap_radius_la_SOURCES = \
	eap_radius_plugin.h eap_radius_plugin.c \
	eap_radius.h eap_radius.c \
	eap_radius_xauth.h eap_radius_xauth.c \
	eap_radius_accounting.h eap_radius_accounting.c \
	eap_radius_provider.h eap_radius_provider.c \
	eap_radius_dae.h eap_radius_dae.c \
	eap_radius_forward.h eap_radius_forward.c

libstrongswan_eap_radius_la_LDFLAGS = -module -avoid-version
