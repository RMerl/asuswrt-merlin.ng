AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtls \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	${xml_CFLAGS} \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-tnc-ifmap.la
else
plugin_LTLIBRARIES = libstrongswan-tnc-ifmap.la
endif

libstrongswan_tnc_ifmap_la_LIBADD = \
	$(top_builddir)/src/libtls/libtls.la ${xml_LIBS}

libstrongswan_tnc_ifmap_la_SOURCES = \
	tnc_ifmap_plugin.h tnc_ifmap_plugin.c \
	tnc_ifmap_listener.h tnc_ifmap_listener.c \
	tnc_ifmap_soap.h tnc_ifmap_soap.c \
	tnc_ifmap_soap_msg.h tnc_ifmap_soap_msg.c \
	tnc_ifmap_http.h tnc_ifmap_http.c \
	tnc_ifmap_renew_session_job.h tnc_ifmap_renew_session_job.c

libstrongswan_tnc_ifmap_la_LDFLAGS = -module -avoid-version
