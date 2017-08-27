AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libhydra \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	${maemo_CFLAGS} \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-maemo.la
else
plugin_LTLIBRARIES = libstrongswan-maemo.la
endif

libstrongswan_maemo_la_SOURCES = \
	maemo_plugin.h maemo_plugin.c \
	maemo_service.h maemo_service.c

libstrongswan_maemo_la_LDFLAGS = -module -avoid-version
libstrongswan_maemo_la_LIBADD  = ${maemo_LIBS}

dbusservice_DATA = org.strongswan.charon.service

org.strongswan.charon.service: $(srcdir)/org.strongswan.charon.service.in
	$(AM_V_GEN) \
	sed -e 's|[@]LIBEXECDIR[@]|$(libexecdir)|' $< >$@

EXTRA_DIST = org.strongswan.charon.service.in
CLEANFILES = $(dbusservice_DATA)
