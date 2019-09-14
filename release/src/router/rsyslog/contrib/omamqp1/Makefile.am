pkglib_LTLIBRARIES = omamqp1.la

omamqp1_la_SOURCES = omamqp1.c
omamqp1_la_CPPFLAGS =  $(RSRT_CFLAGS) $(PTHREADS_CFLAGS) $(PROTON_CFLAGS)
omamqp1_la_LDFLAGS = -module -avoid-version
omamqp1_la_LIBADD = $(PROTON_LIBS) $(PTHREADS_LIBS)

EXTRA_DIST = 
