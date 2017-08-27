# Everything in this directory is statically-linked to libdbus-internal
AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DDBUS_COMPILATION \
	-DDBUS_STATIC_BUILD \
	-DDBUS_TEST_USE_INTERNAL \
	$(NULL)

# if assertions are enabled, improve backtraces
AM_LDFLAGS = @R_DYNAMIC_LDFLAG@

## note that TESTS has special meaning (stuff to use in make check)
## so if adding tests not to be run in make check, don't add them to 
## TESTS
if DBUS_ENABLE_EMBEDDED_TESTS
TESTS_ENVIRONMENT = \
        DBUS_TOP_BUILDDIR=@abs_top_builddir@ \
        DBUS_TOP_SRCDIR=@abs_top_srcdir@ \
        PYTHON=@PYTHON@ \
        DBUS_TEST_DATA=@abs_top_builddir@/test/data \
        DBUS_TEST_DAEMON=@abs_top_builddir@/bus/dbus-daemon$(EXEEXT) \
        XDG_RUNTIME_DIR=@abs_top_builddir@/test/XDG_RUNTIME_DIR \
        $(NULL)

TESTS=run-test.sh run-test-systemserver.sh
else
TESTS=
endif

EXTRA_DIST=run-test.sh run-test-systemserver.sh test-wait-for-echo.py test-activation-forking.py

if DBUS_ENABLE_EMBEDDED_TESTS

## we use noinst_PROGRAMS not check_PROGRAMS for TESTS so that we
## build even when not doing "make check"
noinst_PROGRAMS=test-pending-call-dispatch test-pending-call-timeout test-threads-init test-ids test-shutdown test-privserver test-privserver-client test-autolaunch

test_pending_call_dispatch_LDADD=$(top_builddir)/dbus/libdbus-internal.la
test_pending_call_timeout_LDADD=$(top_builddir)/dbus/libdbus-internal.la
test_threads_init_LDADD=$(top_builddir)/dbus/libdbus-internal.la
test_ids_LDADD=$(top_builddir)/dbus/libdbus-internal.la

test_shutdown_LDADD=../libdbus-testutils-internal.la
test_privserver_LDADD=../libdbus-testutils-internal.la
test_privserver_client_LDADD=../libdbus-testutils-internal.la
test_autolaunch_LDADD=../libdbus-testutils-internal.la

endif
