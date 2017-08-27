## the "name-test" subdir in fact contains a bunch of tests now that need a temporary bus 
## to be running to do stuff with. The directory should be renamed.
## We want to build the current directory first to pick up the testutils lib
SUBDIRS= . name-test 
DIST_SUBDIRS=name-test

# CPPFLAGS for binaries that are normally dynamic
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	$(DBUS_STATIC_BUILD_CPPFLAGS) \
	$(GLIB_CFLAGS) \
	$(DBUS_GLIB_CFLAGS) \
	$(NULL)

# improve backtraces from test stuff
AM_LDFLAGS = @R_DYNAMIC_LDFLAG@

# CPPFLAGS for binaries that are always static
static_cppflags = \
	$(AM_CPPFLAGS) \
	-DDBUS_STATIC_BUILD \
	-DDBUS_COMPILATION \
	-DDBUS_TEST_USE_INTERNAL \
	$(NULL)

noinst_LTLIBRARIES = libdbus-testutils-internal.la

# You can link either libdbus-testutils, dbus-glib and libdbus-1,
# or libdbus-testutils-internal and libdbus-internal - never both in the
# same binary.
if DBUS_WITH_DBUS_GLIB
noinst_LTLIBRARIES += libdbus-testutils.la
libdbus_testutils_la_SOURCES = \
	test-utils.c \
	test-utils.h \
	$(NULL)
libdbus_testutils_la_LIBADD = \
	$(top_builddir)/dbus/libdbus-1.la \
	$(GLIB_LIBS) \
	$(DBUS_GLIB_LIBS) \
	$(NULL)
testutils_shared_if_possible_cppflags = $(AM_CPPFLAGS)
testutils_shared_if_possible_libs = libdbus-testutils.la
else
testutils_shared_if_possible_cppflags = $(static_cppflags)
testutils_shared_if_possible_libs = libdbus-testutils-internal.la
endif

libdbus_testutils_internal_la_CPPFLAGS = \
	$(static_cppflags) \
	$(NULL)
libdbus_testutils_internal_la_SOURCES = \
	test-utils.c \
	test-utils.h \
	$(NULL)
libdbus_testutils_internal_la_LIBADD = \
	$(top_builddir)/dbus/libdbus-internal.la \
	$(NULL)

if DBUS_ENABLE_EMBEDDED_TESTS
## break-loader removed for now
## these binaries are used in tests but are not themselves tests
TEST_BINARIES = \
	test-spawn \
	test-exit \
	test-names \
	test-segfault \
	test-service \
	test-shell-service \
	test-sleep-forever \
	$(NULL)

## These are conceptually part of directories that come earlier in SUBDIRS
## order, but we don't want to run them til we arrive in this directory,
## since they depend on stuff from this directory
TESTS = \
	../bus/test-bus$(EXEEXT) \
	../bus/test-bus-system$(EXEEXT) \
	../dbus/test-dbus$(EXEEXT) \
	$(NULL)

if DBUS_UNIX
TESTS += ../bus/test-bus-launch-helper$(EXEEXT)
endif

else !DBUS_ENABLE_EMBEDDED_TESTS

TEST_BINARIES=
TESTS=

endif !DBUS_ENABLE_EMBEDDED_TESTS

noinst_PROGRAMS= $(TEST_BINARIES)

test_service_CPPFLAGS = $(static_cppflags)
test_service_LDADD = libdbus-testutils-internal.la
test_names_CPPFLAGS = $(static_cppflags)
test_names_LDADD = libdbus-testutils-internal.la
## break_loader_CPPFLAGS = $(static_cppflags)
## break_loader_LDADD = $(top_builddir)/dbus/libdbus-internal.la
test_shell_service_CPPFLAGS = $(static_cppflags)
test_shell_service_LDADD = libdbus-testutils-internal.la
test_shell_SOURCES = shell-test.c
test_shell_CPPFLAGS = $(static_cppflags)
test_shell_LDADD = libdbus-testutils-internal.la
test_spawn_SOURCES = spawn-test.c
test_spawn_CPPFLAGS = $(static_cppflags)
test_spawn_LDADD = $(top_builddir)/dbus/libdbus-internal.la

test_printf_SOURCES = internals/printf.c
test_printf_CPPFLAGS = $(static_cppflags)
test_printf_LDADD = $(top_builddir)/dbus/libdbus-internal.la

test_refs_SOURCES = internals/refs.c
test_refs_CPPFLAGS = $(static_cppflags)
test_refs_LDADD = libdbus-testutils-internal.la $(GLIB_LIBS)

test_syslog_SOURCES = internals/syslog.c
test_syslog_CPPFLAGS = $(static_cppflags)
test_syslog_LDADD = libdbus-testutils-internal.la $(GLIB_LIBS)

EXTRA_DIST = dbus-test-runner

testexecdir = $(libdir)/dbus-1.0/test

testexec_PROGRAMS =

installable_tests = \
	test-shell \
	test-printf \
	$(NULL)
installable_manual_tests = \
	$(NULL)

if DBUS_WITH_GLIB
installable_tests += \
	test-corrupt \
	test-dbus-daemon \
	test-dbus-daemon-eavesdrop \
	test-loopback \
	test-marshal \
	test-refs \
	test-relay \
	test-syntax \
	test-syslog \
	$(NULL)
installable_manual_tests += \
	manual-authz \
	$(NULL)
endif DBUS_WITH_GLIB

installcheck_tests =
installcheck_environment = \
	XDG_RUNTIME_DIR=@abs_top_builddir@/test/XDG_RUNTIME_DIR \
	DBUS_TEST_DAEMON=$(DESTDIR)$(DBUS_DAEMONDIR)/dbus-daemon$(EXEEXT) \
	DBUS_TEST_HOMEDIR=@abs_top_builddir@/dbus \
	DBUS_TEST_SYSCONFDIR=$(DESTDIR)$(sysconfdir)

TESTS_ENVIRONMENT = \
	XDG_RUNTIME_DIR=@abs_top_builddir@/test/XDG_RUNTIME_DIR \
	DBUS_FATAL_WARNINGS=1 \
	DBUS_TEST_DAEMON=@abs_top_builddir@/bus/dbus-daemon$(EXEEXT) \
	DBUS_TEST_DATA=@abs_top_builddir@/test/data \
	DBUS_TEST_HOMEDIR=@abs_top_builddir@/dbus \
	$(NULL)

manual_authz_SOURCES = manual-authz.c
manual_authz_CPPFLAGS = $(testutils_shared_if_possible_cppflags)
manual_authz_LDADD = \
    $(testutils_shared_if_possible_libs) \
    $(GLIB_LIBS) \
    $(NULL)

test_corrupt_SOURCES = corrupt.c
test_corrupt_CPPFLAGS = $(testutils_shared_if_possible_cppflags)
test_corrupt_LDADD = \
    $(testutils_shared_if_possible_libs) \
    $(GLIB_LIBS) \
    $(NULL)

test_loopback_SOURCES = loopback.c
test_loopback_CPPFLAGS = $(testutils_shared_if_possible_cppflags)
test_loopback_LDADD = \
    $(testutils_shared_if_possible_libs) \
    $(GLIB_LIBS) \
    $(NULL)

test_relay_SOURCES = relay.c
test_relay_CPPFLAGS = $(testutils_shared_if_possible_cppflags)
test_relay_LDADD = \
    $(testutils_shared_if_possible_libs) \
    $(GLIB_LIBS) \
    $(NULL)

test_dbus_daemon_SOURCES = dbus-daemon.c
test_dbus_daemon_CPPFLAGS = $(testutils_shared_if_possible_cppflags)
test_dbus_daemon_LDADD = \
    $(testutils_shared_if_possible_libs) \
    $(GLIB_LIBS) \
    $(NULL)

test_dbus_daemon_eavesdrop_SOURCES = dbus-daemon-eavesdrop.c
test_dbus_daemon_eavesdrop_CPPFLAGS = $(testutils_shared_if_possible_cppflags)
test_dbus_daemon_eavesdrop_LDADD = \
    $(testutils_shared_if_possible_libs) \
    $(GLIB_LIBS) \
    $(NULL)

test_marshal_SOURCES = marshal.c
test_marshal_LDADD = \
    $(top_builddir)/dbus/libdbus-1.la \
    $(GLIB_LIBS) \
    $(NULL)

test_syntax_SOURCES = syntax.c
test_syntax_LDADD = \
    $(top_builddir)/dbus/libdbus-1.la \
    $(GLIB_LIBS) \
    $(NULL)

if DBUS_ENABLE_MODULAR_TESTS
TESTS += $(installable_tests)
installcheck_tests += $(installable_tests)

if DBUS_ENABLE_INSTALLED_TESTS
  testexec_PROGRAMS += $(installable_tests) $(installable_manual_tests)
else !DBUS_ENABLE_INSTALLED_TESTS
  noinst_PROGRAMS += $(installable_tests) $(installable_manual_tests)
endif !DBUS_ENABLE_INSTALLED_TESTS

endif DBUS_ENABLE_MODULAR_TESTS

# If we're installing the tests into a DESTDIR we can't run them
# again using the installed copy, because we don't know how to
# do a portable equivalent of setting LD_LIBRARY_PATH.
installcheck-local:
	$(MAKE) check-TESTS TESTS='$$(installcheck_tests)' \
		TESTS_ENVIRONMENT='$$(installcheck_environment)'
if DBUS_ENABLE_INSTALLED_TESTS
	test -n "$(DESTDIR)" || \
	$(installcheck_environment) \
		$(srcdir)/dbus-test-runner \
		$(testexecdir) \
		$(testexec_PROGRAMS)
endif DBUS_ENABLE_INSTALLED_TESTS

in_data = \
	data/valid-config-files-system/debug-allow-all-fail.conf.in \
	data/valid-config-files-system/debug-allow-all-pass.conf.in \
	data/valid-config-files/debug-allow-all-sha1.conf.in \
	data/valid-config-files/debug-allow-all.conf.in \
	data/valid-config-files/incoming-limit.conf.in \
	data/invalid-service-files-system/org.freedesktop.DBus.TestSuiteNoExec.service.in \
	data/invalid-service-files-system/org.freedesktop.DBus.TestSuiteNoService.service.in \
	data/invalid-service-files-system/org.freedesktop.DBus.TestSuiteNoUser.service.in \
	data/valid-service-files-system/org.freedesktop.DBus.TestSuiteEchoService.service.in \
	data/valid-service-files-system/org.freedesktop.DBus.TestSuiteSegfaultService.service.in \
	data/valid-service-files-system/org.freedesktop.DBus.TestSuiteShellEchoServiceFail.service.in \
	data/valid-service-files-system/org.freedesktop.DBus.TestSuiteShellEchoServiceSuccess.service.in \
	data/valid-service-files/org.freedesktop.DBus.TestSuite.PrivServer.service.in \
	data/valid-service-files/org.freedesktop.DBus.TestSuiteEchoService.service.in \
	data/valid-service-files/org.freedesktop.DBus.TestSuiteForkingEchoService.service.in \
	data/valid-service-files/org.freedesktop.DBus.TestSuiteSegfaultService.service.in \
	data/valid-service-files/org.freedesktop.DBus.TestSuiteShellEchoServiceFail.service.in \
	data/valid-service-files/org.freedesktop.DBus.TestSuiteShellEchoServiceSuccess.service.in \
	$(NULL)

EXTRA_DIST += $(in_data)

static_data = \
	name-test/tmp-session-like-system.conf \
	data/auth/anonymous-client-successful.auth-script \
	data/auth/anonymous-server-successful.auth-script \
	data/auth/cancel.auth-script \
	data/auth/client-out-of-mechanisms.auth-script \
	data/auth/external-failed.auth-script \
	data/auth/external-root.auth-script \
	data/auth/external-silly.auth-script \
	data/auth/external-successful.auth-script \
	data/auth/extra-bytes.auth-script \
	data/auth/fail-after-n-attempts.auth-script \
	data/auth/fallback.auth-script \
	data/auth/invalid-command-client.auth-script \
	data/auth/invalid-command.auth-script \
	data/auth/invalid-hex-encoding.auth-script \
	data/auth/mechanisms.auth-script \
	data/equiv-config-files/basic/basic-1.conf \
	data/equiv-config-files/basic/basic-2.conf \
	data/equiv-config-files/basic/basic.d/basic.conf \
	data/equiv-config-files/entities/basic.d/basic.conf \
	data/equiv-config-files/entities/entities-1.conf \
	data/equiv-config-files/entities/entities-2.conf \
	data/incomplete-messages/missing-body.message \
	data/invalid-config-files/badselinux-1.conf \
	data/invalid-config-files/badselinux-2.conf \
	data/invalid-config-files/circular-1.conf \
	data/invalid-config-files/circular-2.conf \
	data/invalid-config-files/circular-3.conf \
	data/invalid-config-files/not-well-formed.conf \
	data/invalid-config-files/truncated-file.conf \
	data/invalid-messages/array-of-nil.message \
	data/invalid-messages/array-with-mixed-types.message \
	data/invalid-messages/bad-boolean-array.message \
	data/invalid-messages/bad-boolean.message \
	data/invalid-messages/bad-endian.message \
	data/invalid-messages/bad-header-field-alignment.message \
	data/invalid-messages/boolean-has-no-value.message-raw \
	data/invalid-messages/local-namespace.message \
	data/invalid-messages/no-dot-in-name.message \
	data/invalid-messages/not-nul-header-padding.message \
	data/invalid-messages/overlong-name.message \
	data/invalid-messages/too-little-header-padding.message \
	data/invalid-messages/too-much-header-padding-by-far.message \
	data/invalid-messages/too-much-header-padding.message \
	data/invalid-messages/too-short-dict.message \
	data/sha-1/Readme.txt \
	data/sha-1/bit-hashes.sha1 \
	data/sha-1/bit-messages.sha1 \
	data/sha-1/byte-hashes.sha1 \
	data/sha-1/byte-messages.sha1 \
	data/valid-config-files/basic.conf \
	data/valid-config-files/basic.d/basic.conf \
	data/valid-config-files/entities.conf \
	data/valid-config-files/many-rules.conf \
	data/valid-config-files/system.d/test.conf \
	data/valid-messages/array-of-array-of-uint32.message \
	data/valid-messages/dict-simple.message \
	data/valid-messages/dict.message \
	data/valid-messages/emptiness.message \
	data/valid-messages/lots-of-arguments.message \
	data/valid-messages/no-padding.message \
	data/valid-messages/opposite-endian.message \
	data/valid-messages/recursive-types.message \
	data/valid-messages/simplest-manual.message \
	data/valid-messages/simplest.message \
	data/valid-messages/standard-acquire-service.message \
	data/valid-messages/standard-hello.message \
	data/valid-messages/standard-list-services.message \
	data/valid-messages/standard-service-exists.message \
	data/valid-messages/unknown-header-field.message \
	$(NULL)

EXTRA_DIST += $(static_data)

## copy tests to builddir so that generated tests and static tests
## are all in one place.
all-local:
	$(AM_V_at)$(MKDIR_P) data/valid-config-files/session.d
	$(AM_V_at)set -e && \
	if test $(srcdir) = . || test $(srcdir) -ef .; then \
		echo '-- No need to copy test data as srcdir = builddir'; \
	else \
		for F in $(static_data); do \
			$(MKDIR_P) $${F%/*}; \
			rm -f $$F; \
			cp $(srcdir)/$$F $$F; \
		done; \
	fi

## this doesn't clean most copied test data files when srcdir=builddir
clean-local:
	$(AM_V_at)if test $(srcdir) = . || test $(srcdir) -ef .; then \
		echo '-- No need to clean test data as srcdir = builddir'; \
	else \
		rm -f $(static_data); \
	fi

imported_data = \
	data/valid-config-files/session.conf \
	data/valid-config-files/system.conf \
	$(NULL)

noinst_DATA = $(imported_data)
CLEANFILES = $(noinst_DATA) XDG_RUNTIME_DIR

$(imported_data): data/valid-config-files/%.conf: $(top_builddir)/bus/%.conf
	$(AM_V_at)$(MKDIR_P) data/valid-config-files
	$(AM_V_GEN)cp $< $@
