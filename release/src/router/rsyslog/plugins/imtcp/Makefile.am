pkglib_LTLIBRARIES = imtcp.la

imtcp_la_SOURCES = imtcp.c
imtcp_la_CPPFLAGS = -I$(top_srcdir) $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS)
imtcp_la_LDFLAGS = -module -avoid-version $(LIBLOGGING_STDLOG_LIBS)
imtcp_la_LIBADD = 
