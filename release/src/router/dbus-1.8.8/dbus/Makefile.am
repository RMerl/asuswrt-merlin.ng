
configdir=$(sysconfdir)/dbus-1

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir) \
	$(SYSTEMD_CFLAGS) \
	$(VALGRIND_CFLAGS) \
	-DDBUS_COMPILATION \
	-DDBUS_MACHINE_UUID_FILE=\""$(localstatedir)/lib/dbus/machine-id"\" \
	-DDBUS_SYSTEM_CONFIG_FILE=\""$(configdir)/system.conf"\" \
	-DDBUS_SESSION_CONFIG_FILE=\""$(configdir)/session.conf"\" \
	$(NULL)

# On Linux with glibc 2.17, sd-daemon.c support for POSIX message queues
# results in an otherwise unnecessary dependency on librt. Disable it.
AM_CPPFLAGS += -DSD_DAEMON_DISABLE_MQ

# if assertions are enabled, improve backtraces
AM_LDFLAGS = @R_DYNAMIC_LDFLAG@

dbusincludedir=$(includedir)/dbus-1.0/dbus
dbusarchincludedir=$(libdir)/dbus-1.0/include/dbus

lib_LTLIBRARIES=libdbus-1.la

#
# Deal with W32 .def and version-info.rc stuff
#
if DBUS_WIN

SUFFIXES = rc

.rc.o:
	$(WINDRES) $< -o $@

dbus_res = versioninfo.o
dbus_res_ldflag = -Wl,$(dbus_res)
no_undefined = -no-undefined
export_symbols =

EXTRA_libdbus_1_la_DEPENDENCIES = $(dbus_res)
intllibs =

else
dbus_res =
dbus_res_ldflag =
no_undefined =
## don't export symbols that start with "_" (we use this
## convention for internal symbols)
export_symbols = -export-symbols-regex "^[^_].*"

intllibs = @LTLIBINTL@

endif


#
# Platform-dependent sources:
#
if DBUS_WIN
DBUS_LIB_arch_sources = 			\
	dbus-server-win.c 			\
	dbus-server-win.h

if DBUS_WINCE
wince_source = dbus-sysdeps-wince-glue.h dbus-sysdeps-wince-glue.c
else
wince_source =
endif

DBUS_SHARED_arch_sources = 			\
	$(wince_source)				\
	dbus-file-win.c 			\
	dbus-pipe-win.c 			\
	dbus-sockets-win.h			\
	dbus-sysdeps-win.c 			\
	dbus-sysdeps-win.h			\
	dbus-sysdeps-thread-win.c		\
	dbus-transport-win.c			\
	dbus-transport-win.h

DBUS_UTIL_arch_sources =			\
	dbus-sysdeps-util-win.c			\
	dbus-spawn-win.c
else

if DBUS_ENABLE_LAUNCHD
launchd_source = dbus-server-launchd.h dbus-server-launchd.c
else
launchd_source =
endif

DBUS_LIB_arch_sources = 			\
	dbus-uuidgen.c				\
	dbus-uuidgen.h				\
	dbus-server-unix.c 			\
	dbus-server-unix.h

DBUS_SHARED_arch_sources = 			\
	$(launchd_source)			\
	dbus-file-unix.c 			\
	dbus-pipe-unix.c 			\
	dbus-sysdeps-unix.c 			\
	dbus-sysdeps-unix.h			\
	dbus-sysdeps-pthread.c			\
	dbus-transport-unix.c			\
	dbus-transport-unix.h			\
	dbus-userdb.c				\
	dbus-userdb.h 				\
	sd-daemon.c                             \
	sd-daemon.h

DBUS_UTIL_arch_sources = 			\
	dbus-sysdeps-util-unix.c		\
	dbus-userdb-util.c			\
	dbus-spawn.c
endif

if HAVE_LINUX_EPOLL
DBUS_UTIL_arch_sources += dbus-socket-set-epoll.c
endif

dbusinclude_HEADERS=				\
	dbus.h					\
	dbus-address.h				\
	dbus-bus.h				\
	dbus-connection.h			\
	dbus-errors.h				\
	dbus-macros.h				\
	dbus-memory.h				\
	dbus-message.h				\
	dbus-misc.h				\
	dbus-pending-call.h			\
	dbus-protocol.h				\
	dbus-server.h				\
	dbus-shared.h				\
	dbus-signature.h			\
	dbus-syntax.h				\
	dbus-threads.h				\
	dbus-types.h


nodist_dbusarchinclude_HEADERS=			\
	dbus-arch-deps.h

### source code that goes in the installed client library
### and is specific to library functionality
DBUS_LIB_SOURCES=				\
	dbus-address.c				\
	dbus-auth.c				\
	dbus-auth.h				\
	dbus-bus.c				\
	dbus-connection.c			\
	dbus-connection-internal.h		\
	dbus-credentials.c			\
	dbus-credentials.h			\
	dbus-errors.c				\
	dbus-keyring.c				\
	dbus-keyring.h				\
	dbus-marshal-header.c			\
	dbus-marshal-header.h			\
	dbus-marshal-byteswap.c			\
	dbus-marshal-byteswap.h			\
	dbus-marshal-recursive.c		\
	dbus-marshal-recursive.h		\
	dbus-marshal-validate.c			\
	dbus-marshal-validate.h			\
	dbus-message.c				\
	dbus-message-internal.h			\
	dbus-message-private.h			\
	dbus-misc.c				\
	dbus-nonce.h				\
	dbus-nonce.c				\
	dbus-object-tree.c			\
	dbus-object-tree.h			\
	dbus-pending-call.c			\
	dbus-pending-call-internal.h		\
	dbus-resources.c			\
	dbus-resources.h			\
	dbus-server.c				\
	dbus-server-debug-pipe.c		\
	dbus-server-debug-pipe.h		\
	dbus-server-protected.h			\
	dbus-server-socket.c			\
	dbus-server-socket.h			\
	$(DBUS_LIB_arch_sources)		\
	dbus-sha.c				\
	dbus-sha.h				\
	dbus-signature.c			\
	dbus-syntax.c				\
	dbus-timeout.c				\
	dbus-timeout.h				\
	dbus-threads-internal.h			\
	dbus-threads.c				\
	dbus-transport.c			\
	dbus-transport.h			\
	dbus-transport-protected.h		\
	dbus-transport-socket.c			\
	dbus-transport-socket.h			\
	dbus-watch.c				\
	dbus-watch.h

### source code that goes in the installed client library
### AND is generic utility functionality used by the
### daemon or test programs (all symbols in here should
### be underscore-prefixed)
DBUS_SHARED_SOURCES=				\
	dbus-dataslot.c				\
	dbus-dataslot.h				\
	dbus-file.c                 \
	dbus-file.h                 \
	dbus-hash.c				\
	dbus-hash.h				\
	dbus-internals.c			\
	dbus-internals.h			\
	dbus-list.c				\
	dbus-list.h				\
	dbus-marshal-basic.c			\
	dbus-marshal-basic.h			\
	dbus-memory.c				\
	dbus-mempool.c				\
	dbus-mempool.h				\
	dbus-pipe.c                 \
	dbus-pipe.h                 \
	dbus-string.c				\
	dbus-string.h				\
	dbus-string-private.h			\
	$(DBUS_SHARED_arch_sources)		\
	dbus-sysdeps.c				\
	dbus-sysdeps.h				\
	dbus-valgrind-internal.h

### source code that is generic utility functionality used
### by the bus daemon or test apps, but is NOT included
### in the D-Bus client library (all symbols in here
### should be underscore-prefixed but don't really need
### to be unless they move to DBUS_SHARED_SOURCES later)
DBUS_UTIL_SOURCES=				\
	dbus-asv-util.c				\
	dbus-asv-util.h				\
	dbus-auth-script.c			\
	dbus-auth-script.h			\
	dbus-auth-util.c			\
	dbus-credentials-util.c			\
	dbus-mainloop.c				\
	dbus-mainloop.h				\
	dbus-marshal-byteswap-util.c		\
	dbus-marshal-recursive-util.c		\
	dbus-marshal-validate-util.c		\
	dbus-message-factory.c			\
	dbus-message-factory.h			\
	dbus-message-util.c			\
	dbus-shell.c				\
	dbus-shell.h				\
	$(DBUS_UTIL_arch_sources)		\
	dbus-socket-set.h			\
	dbus-socket-set.c			\
	dbus-socket-set-poll.c			\
	dbus-spawn.h				\
	dbus-string-util.c			\
	dbus-sysdeps-util.c			\
	dbus-test.c				\
	dbus-test.h

libdbus_1_la_SOURCES=				\
	$(DBUS_LIB_SOURCES)			\
	$(DBUS_SHARED_SOURCES)

libdbus_internal_la_SOURCES=			\
	$(DBUS_LIB_SOURCES)			\
	$(DBUS_SHARED_SOURCES)			\
	$(DBUS_UTIL_SOURCES)

BUILT_SOURCES=$(nodist_dbusarchinclude_HEADERS)
EXTRA_DIST=dbus-arch-deps.h.in

## this library is the same as libdbus, but exports all the symbols
## and is only used for static linking within the dbus package.
noinst_LTLIBRARIES=libdbus-internal.la

libdbus_1_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-Ddbus_1_EXPORTS \
	$(NULL)
libdbus_1_la_LIBADD= $(LIBDBUS_LIBS)
libdbus_1_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	$(export_symbols) \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-no-undefined \
	$(NULL)

libdbus_internal_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DDBUS_STATIC_BUILD \
	$(NULL)
libdbus_internal_la_LIBADD=$(LIBDBUS_LIBS) $(SYSTEMD_LIBS)

if DBUS_WIN
# This must be a separate convenience library, otherwise libtool notices
# that libdbus-1 might contain C++, links it with g++ and links in libstdc++,
# even on Unix where in fact it doesn't contain any C++. For Windows, where
# this code is used, we don't actually need libstdc++.
noinst_LTLIBRARIES += libdbus-init-win.la
libdbus_init_win_la_SOURCES = dbus-init-win.cpp
libdbus_1_la_LIBADD += libdbus-init-win.la
libdbus_internal_la_LIBADD += libdbus-init-win.la
endif

noinst_PROGRAMS =

if DBUS_ENABLE_EMBEDDED_TESTS
# We can't actually run this til we've reached test/
noinst_PROGRAMS += test-dbus
endif

test_dbus_SOURCES=				\
	dbus-test-main.c

test_dbus_LDADD = libdbus-internal.la

## mop up the gcov files
clean-local:
	/bin/rm *.bb *.bbg *.da *.gcov .libs/*.da .libs/*.bbg || true

update-systemd:
	curl http://cgit.freedesktop.org/systemd/systemd/plain/src/libsystemd-daemon/sd-daemon.c > $(srcdir)/sd-daemon.c
	curl http://cgit.freedesktop.org/systemd/systemd/plain/src/systemd/sd-daemon.h > $(srcdir)/sd-daemon.h
