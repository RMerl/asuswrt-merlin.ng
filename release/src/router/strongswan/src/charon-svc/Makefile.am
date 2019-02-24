bin_PROGRAMS = charon-svc

charon_svc_SOURCES = charon-svc.c

charon-svc.o :	$(top_builddir)/config.status

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-DPLUGINS=\""${charon_plugins}\""

charon_svc_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libcharon/libcharon.la
