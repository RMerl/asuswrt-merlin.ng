TESTS = eap_aka_3gpp_tests

check_PROGRAMS = $(TESTS)

eap_aka_3gpp_tests_SOURCES = \
	tests.h tests.c \
	suites/test_vectors.c

eap_aka_3gpp_tests_CFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libstrongswan/tests \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libsimaka \
	-I$(top_srcdir)/src/libcharon/plugins/eap_aka_3gpp \
	-DPLUGINDIR=\""$(abs_top_builddir)/src/libstrongswan/plugins\"" \
	-DPLUGINS=\""${s_plugins}\"" \
	@COVERAGE_CFLAGS@

eap_aka_3gpp_tests_LDFLAGS = @COVERAGE_LDFLAGS@
eap_aka_3gpp_tests_LDADD = \
	../libeap_aka_3gpp.la \
	$(top_builddir)/src/libcharon/libcharon.la \
	$(top_builddir)/src/libstrongswan/tests/libtest.la \
	$(top_builddir)/src/libstrongswan/libstrongswan.la
