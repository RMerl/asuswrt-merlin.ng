pkglib_LTLIBRARIES = imjournal.la
imjournal_la_SOURCES = imjournal.c

imjournal_la_CPPFLAGS = $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBSYSTEMD_JOURNAL_CFLAGS)
#imjournal_la_CPPFLAGS = -I$(top_srcdir) $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBSYSTEMD_JOURNAL_CFLAGS)
imjournal_la_LDFLAGS = -module -avoid-version
imjournal_la_LIBADD =  $(LIBSYSTEMD_JOURNAL_LIBS)
