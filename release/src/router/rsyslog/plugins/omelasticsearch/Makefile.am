pkglib_LTLIBRARIES = omelasticsearch.la

omelasticsearch_la_SOURCES = omelasticsearch.c
omelasticsearch_la_CPPFLAGS = $(RSRT_CFLAGS) $(PTHREADS_CFLAGS) $(CURL_CFLAGS)
omelasticsearch_la_LDFLAGS = -module -avoid-version
omelasticsearch_la_LIBADD =  $(CURL_LIBS) $(LIBM)

EXTRA_DIST = 
