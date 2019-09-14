pkglib_LTLIBRARIES = mmgrok.la

mmgrok_la_SOURCES = mmgrok.c
mmgrok_la_CPPFLAGS = $(GLIB_CFLAGS) $(RSRT_CFLAGS) $(PTHREADS_CFLAGS)
mmgrok_la_LDFLAGS = -module -avoid-version
mmgrok_la_LIBADD = $(GLIB_LIBS) -lgrok $(LIBFASTJSON_LIBS)

EXTRA_DIST = 
