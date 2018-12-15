pkglib_LTLIBRARIES = omgssapi.la

omgssapi_la_SOURCES = omgssapi.c
omgssapi_la_CPPFLAGS = -I$(top_srcdir) $(PTHREADS_CFLAGS) $(RSRT_CFLAGS)
omgssapi_la_LDFLAGS = -module -avoid-version
omgssapi_la_LIBADD = $(GSS_LIBS)
