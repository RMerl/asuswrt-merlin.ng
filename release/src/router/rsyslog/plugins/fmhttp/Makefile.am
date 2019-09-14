pkglib_LTLIBRARIES = fmhttp.la

fmhttp_la_SOURCES = fmhttp.c
fmhttp_la_CPPFLAGS = -I$(top_srcdir) $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(CURL_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS)
fmhttp_la_LDFLAGS = -module -avoid-version
fmhttp_la_LIBADD = $(CURL_LIBS)
