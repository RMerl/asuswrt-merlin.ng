ipsec_PROGRAMS = pt-tls-client

pt_tls_client_SOURCES = pt-tls-client.c

pt-tls-client.o :	$(top_builddir)/config.status

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtls \
	-I$(top_srcdir)/src/libpttls \
	-I$(top_srcdir)/src/libtncif \
	-I$(top_srcdir)/src/libtnccs \
	-DIPSEC_CONFDIR=\"${sysconfdir}\" \
	-DPLUGINS="\"x509 pem pkcs1 pubkey openssl nonce tnc-tnccs tnc-imc tnccs-20\""

pt_tls_client_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libtls/libtls.la \
	$(top_builddir)/src/libpttls/libpttls.la \
	$(top_builddir)/src/libtnccs/libtnccs.la

