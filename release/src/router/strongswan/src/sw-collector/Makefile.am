sbin_PROGRAMS = sw-collector

sw_collector_SOURCES = \
	sw-collector.c \
	sw_collector_db.h sw_collector_db.c \
	sw_collector_dpkg.h sw_collector_dpkg.c \
	sw_collector_history.h sw_collector_history.c \
	sw_collector_rest_api.h sw_collector_rest_api.c

sw-collector.o : $(top_builddir)/config.status

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libimcv \
	-DPLUGINS=\""random openssl wolfssl sqlite curl"\"

AM_CFLAGS = $(json_CFLAGS)

sw_collector_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libimcv/libimcv.la \
	$(json_LIBS)

templatesdir = $(pkgdatadir)/templates/database/sw-collector
dist_templates_DATA = sw_collector_tables.sql

man8_MANS = sw-collector.8

CLEANFILES = $(man8_MANS)


