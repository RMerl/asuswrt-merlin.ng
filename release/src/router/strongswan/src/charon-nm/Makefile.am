ipsec_PROGRAMS = charon-nm

charon_nm_SOURCES = \
	charon-nm.c \
	nm/nm_backend.c nm/nm_backend.h \
	nm/nm_creds.c nm/nm_creds.h \
	nm/nm_handler.c nm/nm_handler.h \
	nm/nm_service.c nm/nm_service.h

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-DIPSEC_DIR=\"${ipsecdir}\" \
	-DIPSEC_PIDDIR=\"${piddir}\" \
	-DNM_CA_DIR=\"${nm_ca_dir}\" \
	-DPLUGINS=\""${nm_plugins}\""

AM_CFLAGS = \
	${nm_CFLAGS}

charon_nm_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libcharon/libcharon.la \
	-lm $(PTHREADLIB) $(ATOMICLIB) $(DLLIB) ${nm_LIBS}

dbusservicedir = $(sysconfdir)/dbus-1/system.d
dbusservice_DATA = nm-strongswan-service.conf

EXTRA_DIST = $(dbusservice_DATA)
