pkglib_LTLIBRARIES = omsnmp.la

omsnmp_la_SOURCES = omsnmp.c omsnmp.h
omsnmp_la_CPPFLAGS = -I$(top_srcdir) $(PTHREADS_CFLAGS) $(RSRT_CFLAGS)
omsnmp_la_LDFLAGS = -module -avoid-version
omsnmp_la_LIBADD =  $(SNMP_LIBS)
