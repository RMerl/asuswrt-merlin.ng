AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-attr-sql.la
else
plugin_LTLIBRARIES = libstrongswan-attr-sql.la
endif

libstrongswan_attr_sql_la_SOURCES = \
	attr_sql_plugin.h attr_sql_plugin.c \
	attr_sql_provider.h attr_sql_provider.c

libstrongswan_attr_sql_la_LDFLAGS = -module -avoid-version
