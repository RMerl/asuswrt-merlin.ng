pkglib_LTLIBRARIES = omtesting.la

omtesting_la_SOURCES = omtesting.c
omtesting_la_CPPFLAGS = -I$(top_srcdir) $(PTHREADS_CFLAGS) $(RSRT_CFLAGS) $(LIBLOGGING_STDLOG_CFLAGS)
omtesting_la_LDFLAGS = -module -avoid-version $(LIBLOGGING_STDLOG_LIBS)
omtesting_la_LIBADD = 
