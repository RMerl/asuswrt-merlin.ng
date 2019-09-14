pkglib_LTLIBRARIES = imuxsock.la

imuxsock_la_SOURCES = imuxsock.c
imuxsock_la_CPPFLAGS = -DSD_EXPORT_SYMBOLS -I$(top_srcdir) $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS)
imuxsock_la_LDFLAGS = -module -avoid-version $(LIBLOGGING_STDLOG_LIBS)
imuxsock_la_LIBADD =
