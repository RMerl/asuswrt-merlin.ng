ipsec_PROGRAMS = scepclient
scepclient_SOURCES = \
scepclient.c scep.c scep.h

scepclient.o :	$(top_builddir)/config.status

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-DIPSEC_CONFDIR=\"${sysconfdir}\" \
	-DPLUGINS=\""${scepclient_plugins}\""

scepclient_LDADD = \
$(top_builddir)/src/libstrongswan/libstrongswan.la

dist_man_MANS = scepclient.8
EXTRA_DIST = Android.mk
