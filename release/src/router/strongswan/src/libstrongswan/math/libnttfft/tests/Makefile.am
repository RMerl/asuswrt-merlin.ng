TESTS = ntt_fft_tests

check_PROGRAMS = $(TESTS)

ntt_fft_tests_SOURCES = \
	suites/test_ntt_fft.c \
	ntt_fft_tests.h ntt_fft_tests.c

ntt_fft_tests_CFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libstrongswan/tests \
	-I$(top_srcdir)/src/libstrongswan/math/libnttfft \
	-DPLUGINDIR=\""$(abs_top_builddir)/src/libstrongswan/plugins\"" \
	-DPLUGINS=\""${s_plugins}\"" \
	@COVERAGE_CFLAGS@

ntt_fft_tests_LDFLAGS = @COVERAGE_LDFLAGS@
ntt_fft_tests_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libstrongswan/tests/libtest.la \
	../libnttfft.la
