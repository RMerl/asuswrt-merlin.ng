pkglib_LTLIBRARIES = imsolaris.la

imsolaris_la_SOURCES = imsolaris.c sun_cddl.c sun_cddl.h imsolaris.h
imsolaris_la_CPPFLAGS = -I$(top_srcdir) $(PTHREADS_CFLAGS) $(RSRT_CFLAGS)
imsolaris_la_LDFLAGS = -module -avoid-version
imsolaris_la_LIBADD = -ldoor -lpthread
