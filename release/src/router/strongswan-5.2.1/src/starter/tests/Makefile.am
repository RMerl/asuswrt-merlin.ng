TESTS = starter_tests

check_PROGRAMS = $(TESTS)

starter_tests_SOURCES = \
	suites/test_parser.c \
	starter_tests.h starter_tests.c

starter_tests_CFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libstrongswan/tests \
	-I$(top_srcdir)/src/starter \
	@COVERAGE_CFLAGS@

starter_tests_LDFLAGS = @COVERAGE_LDFLAGS@
starter_tests_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libstrongswan/tests/libtest.la \
	../libstarter.la
