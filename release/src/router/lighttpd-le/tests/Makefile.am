# lighttpd.conf and conformance.pl expect this directory
testdir=$(srcdir)/tmp/lighttpd/

if CHECK_WITH_FASTCGI
check_PROGRAMS=fcgi-auth fcgi-responder

fcgi_auth_SOURCES=fcgi-auth.c
fcgi_auth_LDADD=-lfcgi

fcgi_responder_SOURCES=fcgi-responder.c
fcgi_responder_LDADD=-lfcgi
endif

TESTS=\
	prepare.sh \
	run-tests.pl \
	cleanup.sh

CONFS=\
	404-handler.conf \
	bug-06.conf \
	bug-12.conf \
	cachable.t \
	condition.conf \
	core-404-handler.t \
	core-condition.t \
	core-keepalive.t \
	core-request.t \
	core-response.t \
	core-var-include.t \
	core.t \
	fastcgi-10.conf \
	fastcgi-13.conf \
	fastcgi-auth.conf \
	fastcgi-responder.conf \
	LightyTest.pm \
	lowercase.conf \
	lowercase.t \
	mod-access.t \
	mod-auth.t \
	mod-cgi.t \
	mod-compress.conf \
	mod-compress.t \
	mod-evhost.conf \
	mod-evhost.t \
	mod-extforward.conf \
	mod-extforward.t \
	mod-fastcgi.t \
	mod-proxy.t \
	mod-redirect.t \
	mod-rewrite.t \
	mod-secdownload.t \
	mod-setenv.t \
	mod-simplevhost.conf \
	mod-simplevhost.t \
	mod-ssi.t \
	mod-userdir.t \
	proxy.conf \
	request.t \
	symlink.t \
	var-include-sub.conf \
	var-include.conf

TESTS_ENVIRONMENT=$(srcdir)/wrapper.sh $(srcdir) $(top_builddir)

EXTRA_DIST=\
	$(CONFS) \
	$(TESTS) \
	CMakeLists.txt \
	lighttpd.conf \
	lighttpd.htpasswd \
	lighttpd.user \
	SConscript \
	wrapper.sh

SUBDIRS=docroot

leak-check:
	for i in $(TESTS); do \
		$(srcdir)/$$i; \
		echo $$?; \
	done

clean-local:
	rm -f *.out
