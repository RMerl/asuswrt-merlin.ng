pkglib_LTLIBRARIES = omjournal.la

omjournal_la_SOURCES = omjournal.c
omjournal_la_CPPFLAGS =  $(RSRT_CFLAGS) $(PTHREADS_CFLAGS) $(LIBSYSTEMD_JOURNAL_CFLAGS)
omjournal_la_LDFLAGS = -module -avoid-version
omjournal_la_LIBADD =  $(LIBSYSTEMD_JOURNAL_LIBS)

EXTRA_DIST = 
