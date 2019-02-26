TESTS = bliss_tests

check_PROGRAMS = $(TESTS)

bliss_tests_SOURCES = \
	suites/test_bliss_bitpacker.c \
	suites/test_bliss_huffman.c \
	suites/test_bliss_keys.c \
	suites/test_bliss_sampler.c \
	suites/test_bliss_signature.c \
	suites/test_bliss_sign.c \
	bliss_tests.h bliss_tests.c

bliss_tests_CFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libstrongswan/tests \
	-I$(top_srcdir)/src/libstrongswan/math/libnttfft \
	-I$(top_srcdir)/src/libstrongswan/plugins/bliss \
	-DPLUGINDIR=\""$(abs_top_builddir)/src/libstrongswan/plugins\"" \
	-DPLUGINS=\""${s_plugins}\"" \
	@COVERAGE_CFLAGS@

bliss_tests_LDFLAGS = @COVERAGE_LDFLAGS@
bliss_tests_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libstrongswan/tests/libtest.la \
	$(top_builddir)/src/libstrongswan/math/libnttfft/libnttfft.la \
	../libbliss.la
