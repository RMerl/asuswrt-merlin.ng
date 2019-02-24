AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I/usr/include/ClearSilver

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

AM_LDFLAGS = \
	-no-undefined

ipseclib_LTLIBRARIES = libfast.la

libfast_la_SOURCES = \
	fast_dispatcher.c fast_request.c fast_session.c fast_smtp.c

if USE_DEV_HEADERS
fast_includedir = ${dev_headers}/fast
nobase_fast_include_HEADERS = \
	fast_context.h fast_controller.h fast_dispatcher.h fast_filter.h \
	fast_request.h fast_session.h fast_smtp.h
endif

libfast_la_LIBADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	-lfcgi $(clearsilver_LIBS) $(PTHREADLIB) $(ATOMICLIB)
