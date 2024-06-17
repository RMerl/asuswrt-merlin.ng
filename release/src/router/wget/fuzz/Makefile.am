AM_CFLAGS = $(WERROR_CFLAGS) $(WARN_CFLAGS) $(CODE_COVERAGE_CFLAGS)
AM_CPPFLAGS = -I$(top_srcdir)/src -I$(srcdir) -I$(top_builddir)/lib -I$(top_srcdir)/lib \
 $(CODE_COVERAGE_CPPFLAGS) \
 -DSRCDIR=\"$(abs_srcdir)\"
LDADD = ../lib/libgnu.a \
 $(GETADDRINFO_LIB) $(HOSTENT_LIB) $(INET_NTOP_LIB) $(INET_PTON_LIB) \
 $(LIBSOCKET) $(LIB_CLOCK_GETTIME) $(LIB_CRYPTO) $(LIB_GETLOGIN) $(LIB_NANOSLEEP) $(LIB_POLL) \
 $(LIB_POSIX_SPAWN) $(LIB_PTHREAD_SIGMASK) $(LIB_SELECT) $(LIBICONV) $(LIBINTL) \
 $(LIBMULTITHREAD) $(LIBTHREAD) $(SERVENT_LIB) @INTL_MACOSX_LIBS@ \
 $(FUZZ_LIBS) $(CODE_COVERAGE_LIBS)

WGET_TESTS = \
 wget_cookie_fuzzer$(EXEEXT) \
 wget_css_fuzzer$(EXEEXT) \
 wget_ftpls_fuzzer$(EXEEXT) \
 wget_html_fuzzer$(EXEEXT) \
 wget_netrc_fuzzer$(EXEEXT) \
 wget_options_fuzzer$(EXEEXT) \
 wget_progress_fuzzer$(EXEEXT) \
 wget_read_hunk_fuzzer$(EXEEXT) \
 wget_robots_fuzzer$(EXEEXT) \
 wget_url_fuzzer$(EXEEXT)

EXTRA_DIST = test-runner.sh

if WITH_NTLM
  WGET_TESTS += wget_ntlm_fuzzer$(EXEEXT)
endif

if FUZZING
  bin_PROGRAMS = $(WGET_TESTS)
  LDADD += $(LIB_FUZZING_ENGINE)
  MAIN = fuzzer.h
#  AM_LDFLAGS = -no-install -all-static
else
  AM_CPPFLAGS += -DTEST_RUN
  AM_TESTS_ENVIRONMENT = export VALGRIND_TESTS"=@VALGRIND_TESTS@";
  LOG_COMPILER = $(top_srcdir)/fuzz/test-runner.sh
  TESTS = $(WGET_TESTS)
  check_PROGRAMS = $(WGET_TESTS)
  MAIN = main.c fuzzer.h
endif

# Make libunittest "PHONY" so we're always sure we're up-to-date.
.PHONY: ../src/libunittest.a
../src/libunittest.a:
	$(MAKE) $(AM_MAKEFLAGS) -C ../src libunittest.a

wget_cookie_fuzzer_SOURCES = wget_cookie_fuzzer.c $(MAIN)
wget_cookie_fuzzer_LDADD = ../src/libunittest.a $(LDADD)

wget_css_fuzzer_SOURCES = wget_css_fuzzer.c $(MAIN)
wget_css_fuzzer_LDADD = ../src/libunittest.a $(LDADD)

wget_ftpls_fuzzer_SOURCES = wget_ftpls_fuzzer.c $(MAIN)
wget_ftpls_fuzzer_LDADD = ../src/libunittest.a $(LDADD)

wget_html_fuzzer_SOURCES = wget_html_fuzzer.c $(MAIN)
wget_html_fuzzer_LDADD = ../src/libunittest.a $(LDADD)

wget_netrc_fuzzer_SOURCES = wget_netrc_fuzzer.c $(MAIN)
wget_netrc_fuzzer_LDADD = ../src/libunittest.a $(LDADD)

wget_ntlm_fuzzer_SOURCES = wget_ntlm_fuzzer.c $(MAIN)
wget_ntlm_fuzzer_LDADD = ../src/libunittest.a $(LDADD)

wget_options_fuzzer_SOURCES = wget_options_fuzzer.c $(MAIN)
wget_options_fuzzer_LDADD = ../src/libunittest.a $(LDADD)

wget_progress_fuzzer_SOURCES = wget_progress_fuzzer.c $(MAIN)
wget_progress_fuzzer_LDADD = ../src/libunittest.a $(LDADD)

wget_read_hunk_fuzzer_SOURCES = wget_read_hunk_fuzzer.c $(MAIN)
wget_read_hunk_fuzzer_LDADD = ../src/libunittest.a $(LDADD)

wget_robots_fuzzer_SOURCES = wget_robots_fuzzer.c $(MAIN)
wget_robots_fuzzer_LDADD = ../src/libunittest.a $(LDADD)

wget_url_fuzzer_SOURCES = wget_url_fuzzer.c $(MAIN)
wget_url_fuzzer_LDADD = ../src/libunittest.a $(LDADD)

#EXTRA_DIST = $(wildcard *.options) $(wildcard *.dict) \
# $(wildcard *.in) $(wildcard *.repro)

dist-hook:
	find $(srcdir) -name '*.options' -exec cp -v '{}' $(distdir) ';'
	find $(srcdir) -name '*.dict' -exec cp -v '{}' $(distdir) ';'
#	find $(srcdir) -name '*.in' -exec cp -vr '{}' $(distdir) ';'
	find $(srcdir) -name '*.repro' -exec cp -vr '{}' $(distdir) ';'

clean-local:
	rm -rf *.gc?? *.log lcov

oss-fuzz:
	if test "$$OUT" != ""; then \
	XLIBS="-lpsl -lgnutls -lhogweed -lnettle -lidn2 -lunistring -lz"; \
	for ccfile in wget*_fuzzer.c; do \
	fuzzer=$$(basename $$ccfile .c); \
	$$CC $$CFLAGS -I$(top_srcdir)/src -I$(top_srcdir) -I$(top_srcdir)/lib \
	"$${fuzzer}.c" -o "$${fuzzer}" \
	../src/libunittest.a ../lib/libgnu.a $${LIB_FUZZING_ENGINE} \
	-Wl,-Bstatic $${XLIBS} -Wl,-Bdynamic; \
	done; \
	fi

.PHONY: oss-fuzz
