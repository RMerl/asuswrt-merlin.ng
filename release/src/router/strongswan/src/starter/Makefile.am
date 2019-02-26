SUBDIRS = . tests

ipsec_PROGRAMS = starter
starter_SOURCES = \
starter.c files.h \
args.c args.h confread.c confread.h keywords.c keywords.h cmp.c cmp.h \
invokecharon.c invokecharon.h starterstroke.c starterstroke.h \
netkey.c netkey.h klips.c klips.h

# parser is also used by tests
noinst_LTLIBRARIES = libstarter.la
libstarter_la_SOURCES = \
parser/parser.y parser/lexer.l parser/conf_parser.c parser/conf_parser.h

AM_CPPFLAGS = \
	-I${linux_headers} \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/starter \
	-I$(top_srcdir)/src/stroke \
	-DIPSEC_DIR=\"${ipsecdir}\" \
	-DIPSEC_CONFDIR=\"${sysconfdir}\" \
	-DIPSEC_PIDDIR=\"${piddir}\" \
	-DIPSEC_EAPDIR=\"${eapdir}\" \
	-DIPSEC_SCRIPT=\"${ipsec_script}\" \
	-DDEV_RANDOM=\"${random_device}\" \
	-DDEV_URANDOM=\"${urandom_device}\" \
	-DPLUGINS=\""${starter_plugins}\"" \
	-DDEBUG

AM_CFLAGS = \
	@COVERAGE_CFLAGS@

AM_YFLAGS = -v -d

starter_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libcharon/libcharon.la \
	libstarter.la \
	$(SOCKLIB) $(PTHREADLIB) $(ATOMICLIB)

EXTRA_DIST = keywords.h.in keywords.txt ipsec.conf ipsec.secrets Android.mk
MAINTAINERCLEANFILES = keywords.h keywords.c
BUILT_SOURCES = keywords.h keywords.c parser/parser.h

if USE_LOAD_WARNING
  AM_CPPFLAGS += -DLOAD_WARNING
endif

keywords.h: $(srcdir)/keywords.h.in
		$(AM_V_GEN) \
		sed \
		-e "s:\@GPERF_LEN_TYPE\@:$(GPERF_LEN_TYPE):" \
		$< > $@

keywords.c:	$(srcdir)/keywords.txt keywords.h
		$(AM_V_GEN) \
		$(GPERF) -m 10 -C -G -D -t < $(srcdir)/keywords.txt > $@

install-exec-local :
		test -e "$(DESTDIR)${sysconfdir}/ipsec.d" || $(INSTALL) -d "$(DESTDIR)$(sysconfdir)/ipsec.d" || true
		test -e "$(DESTDIR)${sysconfdir}/ipsec.d/cacerts" || $(INSTALL) -d "$(DESTDIR)$(sysconfdir)/ipsec.d/cacerts" || true
		test -e "$(DESTDIR)${sysconfdir}/ipsec.d/ocspcerts" || $(INSTALL) -d "$(DESTDIR)$(sysconfdir)/ipsec.d/ocspcerts" || true
		test -e "$(DESTDIR)${sysconfdir}/ipsec.d/certs" || $(INSTALL) -d "$(DESTDIR)$(sysconfdir)/ipsec.d/certs" || true
		test -e "$(DESTDIR)${sysconfdir}/ipsec.d/acerts" || $(INSTALL) -d "$(DESTDIR)$(sysconfdir)/ipsec.d/acerts" || true
		test -e "$(DESTDIR)${sysconfdir}/ipsec.d/aacerts" || $(INSTALL) -d "$(DESTDIR)$(sysconfdir)/ipsec.d/aacerts" || true
		test -e "$(DESTDIR)${sysconfdir}/ipsec.d/crls" || $(INSTALL) -d "$(DESTDIR)$(sysconfdir)/ipsec.d/crls" || true
		test -e "$(DESTDIR)${sysconfdir}/ipsec.d/reqs" || $(INSTALL) -d "$(DESTDIR)$(sysconfdir)/ipsec.d/reqs" || true
		test -e "$(DESTDIR)${sysconfdir}/ipsec.d/private" || $(INSTALL) -d -m 750 "$(DESTDIR)$(sysconfdir)/ipsec.d/private" || true
		test -e "$(DESTDIR)$(sysconfdir)/ipsec.conf" || $(INSTALL) -m 644 $(srcdir)/ipsec.conf $(DESTDIR)$(sysconfdir)/ipsec.conf || true
		test -e "$(DESTDIR)$(sysconfdir)/ipsec.secrets" || $(INSTALL) -m 600 $(srcdir)/ipsec.secrets $(DESTDIR)$(sysconfdir)/ipsec.secrets || true
