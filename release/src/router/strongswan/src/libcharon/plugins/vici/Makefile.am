AM_CPPFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libstrongswan/plugins/pubkey \
	-I$(top_srcdir)/src/libcharon \
	-I$(top_srcdir)/src/libcharon/plugins/counters \
	-DSWANCTLDIR=\""${swanctldir}\"" \
	-DIPSEC_PIDDIR=\"${piddir}\"

AM_CFLAGS = \
	$(PLUGIN_CFLAGS)

if MONOLITHIC
noinst_LTLIBRARIES = libstrongswan-vici.la
else
plugin_LTLIBRARIES = libstrongswan-vici.la
endif

libstrongswan_vici_la_SOURCES = \
	vici_socket.h vici_socket.c \
	vici_message.h vici_message.c \
	vici_builder.h vici_builder.c \
	vici_dispatcher.h vici_dispatcher.c \
	vici_cert_info.h vici_cert_info.c \
	vici_query.h vici_query.c \
	vici_control.h vici_control.c \
	vici_config.h vici_config.c \
	vici_cred.h vici_cred.c \
	vici_attribute.h vici_attribute.c \
	vici_authority.h vici_authority.c \
	vici_logger.h vici_logger.c \
	vici_plugin.h vici_plugin.c

libstrongswan_vici_la_LDFLAGS = -module -avoid-version


EXTRA_DIST = README.md


ipseclib_LTLIBRARIES = libvici.la

libvici_la_SOURCES = \
	vici_message.c vici_message.h \
	vici_builder.c vici_builder.h \
	vici_cert_info.h vici_cert_info.c \
	libvici.c libvici.h

libvici_la_LIBADD = $(top_builddir)/src/libstrongswan/libstrongswan.la


TESTS = vici_tests

check_PROGRAMS = $(TESTS)

vici_tests_SOURCES = \
	suites/test_socket.c \
	suites/test_message.c \
	suites/test_request.c \
	suites/test_event.c \
	vici_socket.c \
	vici_message.c \
	vici_builder.c \
	vici_dispatcher.c \
	libvici.c \
	vici_tests.h vici_tests.c

vici_tests_CFLAGS = \
	-I$(top_srcdir)/src/libstrongswan \
	-I$(top_srcdir)/src/libstrongswan/tests \
	@COVERAGE_CFLAGS@

vici_tests_LDFLAGS = @COVERAGE_LDFLAGS@
vici_tests_LDADD = \
	$(top_builddir)/src/libstrongswan/libstrongswan.la \
	$(top_builddir)/src/libstrongswan/tests/libtest.la


SUBDIRS =

if USE_RUBY_GEMS
SUBDIRS += ruby
endif

if USE_PYTHON_EGGS
SUBDIRS += python
endif

if USE_PERL_CPAN
SUBDIRS += perl
endif
