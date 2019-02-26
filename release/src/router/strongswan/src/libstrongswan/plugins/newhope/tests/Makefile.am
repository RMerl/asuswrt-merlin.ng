TESTS = newhope_tests

check_PROGRAMS = $(TESTS)

newhope_tests_SOURCES = \
	suites/test_newhope_ke.c \
	suites/test_newhope_noise.c \
	suites/test_newhope_reconciliation.c \
	newhope_tests.h newhope_tests.c

newhope_tests_CFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libstrongswan/tests \
	-I$(top_srcdir)/src/libstrongswan/math/libnttfft \
	-I$(top_srcdir)/src/libstrongswan/plugins/newhope \
	-DPLUGINDIR=\""$(abs_top_builddir)/src/libstrongswan/plugins\"" \
	-DPLUGINS=\""${s_plugins}\"" \
	@COVERAGE_CFLAGS@

newhope_tests_LDFLAGS = @COVERAGE_LDFLAGS@
newhope_tests_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libstrongswan/tests/libtest.la \
	$(top_builddir)/src/libstrongswan/math/libnttfft/libnttfft.la \
	../libnewhope.la
