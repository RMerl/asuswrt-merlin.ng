pkglib_LTLIBRARIES = ompgsql.la

ompgsql_la_SOURCES = ompgsql.c
ompgsql_la_CPPFLAGS = -I$(top_srcdir) $(PGSQL_CFLAGS) $(RSRT_CFLAGS)
ompgsql_la_LDFLAGS = -module -avoid-version
ompgsql_la_LIBADD = $(PGSQL_LIBS)

EXTRA_DIST = createDB.sql
