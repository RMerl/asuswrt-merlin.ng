pkglib_LTLIBRARIES = omlibdbi.la

omlibdbi_la_SOURCES = omlibdbi.c
omlibdbi_la_CPPFLAGS = -I$(top_srcdir) $(LIBDBI_CFLAGS) $(PTHREADS_CFLAGS) $(RSRT_CFLAGS)
omlibdbi_la_LDFLAGS = -module -avoid-version
omlibdbi_la_LIBADD = $(LIBDBI_LIBS)
