TESTS = tls_tests

check_PROGRAMS = $(TESTS)

tls_tests_SOURCES = \
	suites/test_socket.c \
	suites/test_suites.c \
	tls_tests.h tls_tests.c

tls_tests_CFLAGS = \
	-I$(top_srcdir)/src/libtls \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libstrongswan/tests \
	-DPLUGINDIR=\""$(abs_top_builddir)/src/libstrongswan/plugins\"" \
	-DPLUGINS=\""${s_plugins}\"" \
	@COVERAGE_CFLAGS@

tls_tests_LDFLAGS = @COVERAGE_LDFLAGS@
tls_tests_LDADD = \
	$(top_builddir)/src/libtls/libtls.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libstrongswan/tests/libtest.la
