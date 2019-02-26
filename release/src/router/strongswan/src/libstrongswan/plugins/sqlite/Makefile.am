AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-sqlite.la
else
plugin_LTLIBRARIES = libstrongswan-sqlite.la
endif

libstrongswan_sqlite_la_SOURCES = \
	sqlite_plugin.h sqlite_plugin.c \
	sqlite_database.h sqlite_database.c

libstrongswan_sqlite_la_LDFLAGS = -module -avoid-version
libstrongswan_sqlite_la_LIBADD  = -lsqlite3
