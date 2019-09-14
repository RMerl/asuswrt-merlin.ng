#
# fmhash support
# 
pkglib_LTLIBRARIES = fmhash.la
fmhash_la_SOURCES = fmhash.c
fmhash_la_CPPFLAGS = $(PTHREADS_CFLAGS) $(RSRT_CFLAGS)
fmhash_la_LDFLAGS = -module -avoid-version
fmhash_la_LIBADD = $(HASH_XXHASH_LIBS)
