AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtls \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libtnccs

AM_LDFLAGS = \
	-no-undefined

ipseclib_LTLIBRARIES = libpttls.la

libpttls_la_LIBADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libtls/libtls.la

if USE_WINDOWS
  libpttls_la_LIBADD += -lws2_32
endif

libpttls_la_SOURCES = pt_tls.c pt_tls.h \
	pt_tls_client.c pt_tls_client.h \
	pt_tls_server.c pt_tls_server.h \
	pt_tls_dispatcher.c pt_tls_dispatcher.h \
	sasl/sasl_plain/sasl_plain.c sasl/sasl_plain/sasl_plain.h \
	sasl/sasl_mechanism.c sasl/sasl_mechanism.h
