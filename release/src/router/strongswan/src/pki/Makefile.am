SUBDIRS = man

bin_PROGRAMS = pki

pki_SOURCES = pki.c pki.h pki_cert.c pki_cert.h command.c command.h \
	commands/acert.c \
	commands/dn.c \
	commands/est.c \
	commands/estca.c \
	commands/gen.c \
	commands/issue.c \
	commands/keyid.c \
	commands/ocsp.c \
	commands/pkcs12.c \
	commands/pkcs7.c \
	commands/print.c \
	commands/pub.c \
	commands/req.c \
	commands/scep.c \
	commands/scepca.c \
	commands/self.c \
	commands/signcrl.c \
	commands/verify.c \
	est/est_tls.h est/est_tls.c	\
	ocsp/index_responder.h ocsp/index_responder.c	\
	scep/scep.h scep/scep.c

pki_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libtls/libtls.la \
	$(PTHREADLIB) $(ATOMICLIB) $(DLLIB)

pki.o :	$(top_builddir)/config.status

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libtls \
	-DPLUGINS=\""${pki_plugins}\""
