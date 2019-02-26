TESTS = ipsec_tests

check_PROGRAMS = $(TESTS)

ipsec_tests_SOURCES = \
	suites/test_chapoly.c \
	ipsec_tests.h ipsec_tests.c

ipsec_tests_CFLAGS = \
	-I$(top_srcdir)/src/libipsec \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libstrongswan/tests \
	-DPLUGINDIR=\""$(abs_top_builddir)/src/libstrongswan/plugins\"" \
	-DPLUGINS=\""${s_plugins}\"" \
	@COVERAGE_CFLAGS@

ipsec_tests_LDFLAGS = @COVERAGE_LDFLAGS@
ipsec_tests_LDADD = \
	$(top_builddir)/src/libipsec/libipsec.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libstrongswan/tests/libtest.la
