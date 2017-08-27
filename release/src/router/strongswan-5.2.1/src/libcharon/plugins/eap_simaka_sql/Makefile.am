AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libsimaka \
	-DIPSEC_CONFDIR=\"${sysconfdir}\"

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-eap-simaka-sql.la
else
plugin_LTLIBRARIES = libstrongswan-eap-simaka-sql.la
libstrongswan_eap_simaka_sql_la_LIBADD = $(top_builddir)/src/libsimaka/libsimaka.la
endif

libstrongswan_eap_simaka_sql_la_SOURCES = \
	eap_simaka_sql_plugin.h eap_simaka_sql_plugin.c \
	eap_simaka_sql_card.h eap_simaka_sql_card.c \
	eap_simaka_sql_provider.h eap_simaka_sql_provider.c

libstrongswan_eap_simaka_sql_la_LDFLAGS = -module -avoid-version
