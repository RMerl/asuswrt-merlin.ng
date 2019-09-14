pkglib_LTLIBRARIES = immark.la

immark_la_SOURCES = immark.c immark.h
immark_la_CPPFLAGS = $(RSRT_CFLAGS) -I$(top_srcdir) $(PTHREADS_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS)
immark_la_LDFLAGS = -module -avoid-version $(LIBLOGGING_STDLOG_LIBS)
immark_la_LIBADD = 
