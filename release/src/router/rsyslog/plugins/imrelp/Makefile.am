pkglib_LTLIBRARIES = imrelp.la

imrelp_la_SOURCES = imrelp.c
imrelp_la_CPPFLAGS = -I$(top_srcdir) $(PTHREADS_CFLAGS) $(RELP_CFLAGS) $(RSRT_CFLAGS)
imrelp_la_LDFLAGS = -module -avoid-version
imrelp_la_LIBADD = $(RELP_LIBS)
