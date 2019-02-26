AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-sql.la
else
plugin_LTLIBRARIES = libstrongswan-sql.la
endif

libstrongswan_sql_la_SOURCES = \
	sql_plugin.h sql_plugin.c sql_config.h sql_config.c \
	sql_cred.h sql_cred.c sql_logger.h sql_logger.c

libstrongswan_sql_la_LDFLAGS = -module -avoid-version
