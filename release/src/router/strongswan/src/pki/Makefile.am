SUBDIRS = man

bin_PROGRAMS = pki

pki_SOURCES = pki.c pki.h command.c command.h \
	commands/acert.c \
	commands/dn.c \
	commands/gen.c \
	commands/issue.c \
	commands/keyid.c \
	commands/pkcs12.c \
	commands/pkcs7.c \
	commands/print.c \
	commands/pub.c \
	commands/req.c \
	commands/self.c \
	commands/signcrl.c \
	commands/verify.c

pki_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(PTHREADLIB) $(ATOMICLIB) $(DLLIB)

pki.o :	$(top_builddir)/config.status

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-DPLUGINS=\""${pki_plugins}\""
