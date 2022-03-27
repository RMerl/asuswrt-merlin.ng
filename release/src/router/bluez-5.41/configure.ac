AC_PREREQ(2.60)
AC_INIT(bluez, 5.41)

AM_INIT_AUTOMAKE([foreign subdir-objects color-tests silent-rules
					tar-pax no-dist-gzip dist-xz])
AC_CONFIG_HEADERS(config.h)
AC_USE_SYSTEM_EXTENSIONS

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_MAINTAINER_MODE

AC_PREFIX_DEFAULT(/usr/local)

PKG_PROG_PKG_CONFIG

COMPILER_FLAGS

AC_LANG_C

AC_C_RESTRICT

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CC_PIE
AC_PROG_INSTALL
AC_PROG_MKDIR_P

m4_define([_LT_AC_TAGCONFIG], [])
m4_ifdef([AC_LIBTOOL_TAGS], [AC_LIBTOOL_TAGS([])])

AC_DISABLE_STATIC
AC_PROG_LIBTOOL

if (test "$USE_MAINTAINER_MODE" = "yes"); then
	AC_CHECK_PROG(enable_coverage, [lcov], [yes], [no])
	AC_CHECK_PROG(enable_dbus_run_session, [dbus-run-session], [yes])
	AC_CHECK_PROG(enable_valgrind, [valgrind], [yes])
	AC_CHECK_HEADERS(valgrind/memcheck.h)
fi
AM_CONDITIONAL(COVERAGE, test "${enable_coverage}" = "yes")
AM_CONDITIONAL(DBUS_RUN_SESSION, test "${enable_dbus_run_session}" = "yes")
AM_CONDITIONAL(VALGRIND, test "${enable_valgrind}" = "yes")

MISC_FLAGS

AC_ARG_ENABLE(threads, AC_HELP_STRING([--enable-threads],
		[enable threading support]), [enable_threads=${enableval}])

AC_CHECK_FUNC(signalfd, dummy=yes,
			AC_MSG_ERROR(signalfd support is required))

AC_CHECK_LIB(rt, clock_gettime, dummy=yes,
			AC_MSG_ERROR(realtime clock support is required))

AC_CHECK_LIB(pthread, pthread_create, dummy=yes,
			AC_MSG_ERROR(posix thread support is required))

AC_CHECK_LIB(dl, dlopen, dummy=yes,
			AC_MSG_ERROR(dynamic linking loader is required))

AC_CHECK_HEADERS(linux/types.h linux/if_alg.h)

PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.28, dummy=yes,
				AC_MSG_ERROR(GLib >= 2.28 is required))
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

if (test "${enable_threads}" = "yes"); then
	AC_DEFINE(NEED_THREADS, 1, [Define if threading support is required])
	PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.16, dummy=yes,
				AC_MSG_ERROR(GThread >= 2.16 is required))
	GLIB_CFLAGS="$GLIB_CFLAGS $GTHREAD_CFLAGS"
	GLIB_LIBS="$GLIB_LIBS $GTHREAD_LIBS"
fi

PKG_CHECK_MODULES(DBUS, dbus-1 >= 1.6, dummy=yes,
				AC_MSG_ERROR(D-Bus >= 1.6 is required))
AC_SUBST(DBUS_CFLAGS)
AC_SUBST(DBUS_LIBS)

AC_ARG_WITH([dbusconfdir], AC_HELP_STRING([--with-dbusconfdir=DIR],
				[path to D-Bus configuration directory]),
					[path_dbusconfdir=${withval}])
if (test -z "${path_dbusconfdir}"); then
	AC_MSG_CHECKING([D-Bus configuration directory])
	path_dbusconfdir="`$PKG_CONFIG --variable=sysconfdir dbus-1`"
	if (test -z "${path_dbusconfdir}"); then
		AC_MSG_ERROR([D-Bus configuration directory is required])
	fi
	AC_MSG_RESULT([${path_dbusconfdir}])
fi
AC_SUBST(DBUS_CONFDIR, [${path_dbusconfdir}])

AC_ARG_WITH([dbussystembusdir], AC_HELP_STRING([--with-dbussystembusdir=DIR],
				[path to D-Bus system bus services directory]),
					[path_dbussystembusdir=${withval}])
if (test -z "${path_dbussystembusdir}"); then
	AC_MSG_CHECKING([D-Bus system bus services dir])
	path_dbussystembusdir="`$PKG_CONFIG --variable=system_bus_services_dir dbus-1`"
	if (test -z "${path_dbussystembusdir}"); then
		AC_MSG_ERROR([D-Bus system bus services directory is required])
	fi
	AC_MSG_RESULT([${path_dbussystembusdir}])
fi
AC_SUBST(DBUS_SYSTEMBUSDIR, [${path_dbussystembusdir}])

AC_ARG_WITH([dbussessionbusdir], AC_HELP_STRING([--with-dbussessionbusdir=DIR],
				[path to D-Bus session bus services directory]),
					[path_dbussessionbusdir=${withval}])
if (test -z "${path_dbussessionbusdir}"); then
	AC_MSG_CHECKING([D-Bus session bus services dir])
	path_dbussessionbusdir="`$PKG_CONFIG --variable=session_bus_services_dir dbus-1`"
	if (test -z "${path_dbussessionbusdir}"); then
		AC_MSG_ERROR([D-Bus session bus services directory is required])
	fi
	AC_MSG_RESULT([${path_dbussessionbusdir}])
fi
AC_SUBST(DBUS_SESSIONBUSDIR, [${path_dbussessionbusdir}])

AC_ARG_ENABLE(backtrace, AC_HELP_STRING([--enable-backtrace],
		[compile backtrace support]), [enable_backtrace=${enableval}])

if (test "${enable_backtrace}" = "yes"); then
	AC_CHECK_HEADER(elfutils/libdwfl.h, dummy=yes,
			AC_MSG_ERROR(elfutils support is required))
	AC_DEFINE(HAVE_BACKTRACE_SUPPORT, 1,
			[Define to 1 if you have the backtrace support.])
	BACKTRACE_CFLAGS=""
	BACKTRACE_LIBS="-ldw"
	AC_SUBST(BACKTRACE_CFLAGS)
	AC_SUBST(BACKTRACE_LIBS)
fi

AC_ARG_ENABLE(library, AC_HELP_STRING([--enable-library],
		[install Bluetooth library]), [enable_library=${enableval}])
AM_CONDITIONAL(LIBRARY, test "${enable_library}" = "yes")

AC_ARG_ENABLE(test, AC_HELP_STRING([--enable-test],
		[enable test/example scripts]), [enable_test=${enableval}])
AM_CONDITIONAL(TEST, test "${enable_test}" = "yes")

AC_ARG_ENABLE(tools, AC_HELP_STRING([--disable-tools],
		[disable Bluetooth tools]), [enable_tools=${enableval}])
AM_CONDITIONAL(TOOLS, test "${enable_tools}" != "no")

AC_ARG_ENABLE(monitor, AC_HELP_STRING([--disable-monitor],
		[disable Bluetooth monitor]), [enable_monitor=${enableval}])
AM_CONDITIONAL(MONITOR, test "${enable_monitor}" != "no")

AC_ARG_ENABLE(udev, AC_HELP_STRING([--disable-udev],
		[disable udev device support]), [enable_udev=${enableval}])
if (test "${enable_tools}" != "no" && test "${enable_udev}" != "no"); then
	PKG_CHECK_MODULES(UDEV, libudev >= 172, dummy=yes,
				AC_MSG_ERROR(libudev >= 172 is required))
	AC_SUBST(UDEV_CFLAGS)
	AC_SUBST(UDEV_LIBS)
	AC_CHECK_LIB(udev, udev_hwdb_new,
		AC_DEFINE(HAVE_UDEV_HWDB_NEW, 1,
			[Define to 1 if you have the udev_hwdb_new() function.]))
fi
AM_CONDITIONAL(UDEV, test "${enable_udev}" != "no")

AC_ARG_WITH([udevdir], AC_HELP_STRING([--with-udevdir=DIR],
			[path to udev directory]), [path_udevdir=${withval}])
if (test "${enable_udev}" != "no" && test -z "${path_udevdir}"); then
	AC_MSG_CHECKING([udev directory])
	path_udevdir="`$PKG_CONFIG --variable=udevdir udev`"
	if (test -z "${path_udevdir}"); then
		AC_MSG_ERROR([udev directory is required])
	fi
	AC_MSG_RESULT([${path_udevdir}])
fi
AC_SUBST(UDEV_DIR, [${path_udevdir}])

AM_CONDITIONAL(HID2HCI, test "${enable_tools}" != "no" &&
						test "${enable_udev}" != "no")

AC_ARG_ENABLE(cups, AC_HELP_STRING([--disable-cups],
                [disable CUPS printer support]), [enable_cups=${enableval}])
AM_CONDITIONAL(CUPS, test "${enable_cups}" != "no")

AC_ARG_ENABLE(obex, AC_HELP_STRING([--disable-obex],
		[disable OBEX profile support]), [enable_obex=${enableval}])
if (test "${enable_obex}" != "no"); then
	PKG_CHECK_MODULES(ICAL, libical, dummy=yes,
					AC_MSG_ERROR(libical is required))
	AC_SUBST(ICAL_CFLAGS)
	AC_SUBST(ICAL_LIBS)
fi
AM_CONDITIONAL(OBEX, test "${enable_obex}" != "no")

AC_ARG_ENABLE(client, AC_HELP_STRING([--disable-client],
		[disable command line client]), [enable_client=${enableval}])
AM_CONDITIONAL(CLIENT, test "${enable_client}" != "no")

if (test "${enable_client}" != "no"); then
        AC_CHECK_HEADERS(readline/readline.h, enable_readline=yes,
                AC_MSG_ERROR(readline header files are required))
fi
AM_CONDITIONAL(READLINE, test "${enable_readline}" = "yes")

AC_ARG_ENABLE(systemd, AC_HELP_STRING([--disable-systemd],
		[disable systemd integration]), [enable_systemd=${enableval}])
AM_CONDITIONAL(SYSTEMD, test "${enable_systemd}" != "no")

AC_ARG_WITH([systemdsystemunitdir],
			AC_HELP_STRING([--with-systemdsystemunitdir=DIR],
			[path to systemd system unit directory]),
					[path_systemunitdir=${withval}])
if (test "${enable_systemd}" != "no" && test -z "${path_systemunitdir}"); then
	AC_MSG_CHECKING([systemd system unit dir])
	path_systemunitdir="`$PKG_CONFIG --variable=systemdsystemunitdir systemd`"
	if (test -z "${path_systemunitdir}"); then
		AC_MSG_ERROR([systemd system unit directory is required])
	fi
	AC_MSG_RESULT([${path_systemunitdir}])
fi
AC_SUBST(SYSTEMD_SYSTEMUNITDIR, [${path_systemunitdir}])

AC_ARG_WITH([systemduserunitdir],
			AC_HELP_STRING([--with-systemduserunitdir=DIR],
			[path to systemd user unit directory]),
					[path_userunitdir=${withval}])
if (test "${enable_systemd}" != "no" && test -z "${path_userunitdir}"); then
	AC_MSG_CHECKING([systemd user unit dir])
	path_userunitdir="`$PKG_CONFIG --variable=systemduserunitdir systemd`"
	if (test -z "${path_userunitdir}"); then
		AC_MSG_ERROR([systemd user unit directory is required])
	fi
	AC_MSG_RESULT([${path_userunitdir}])
fi
AC_SUBST(SYSTEMD_USERUNITDIR, [${path_userunitdir}])

AC_ARG_ENABLE(datafiles, AC_HELP_STRING([--disable-datafiles],
			[do not install configuration and data files]),
					[enable_datafiles=${enableval}])
AM_CONDITIONAL(DATAFILES, test "${enable_datafiles}" != "no")

AC_ARG_ENABLE(manpages, AC_HELP_STRING([--enable-manpages],
			[enable building of manual pages]),
					[enable_manpages=${enableval}])
AM_CONDITIONAL(MANPAGES, test "${enable_manpages}" = "yes")

AC_ARG_ENABLE(experimental, AC_HELP_STRING([--enable-experimental],
			[enable experimental plugins (SAP, NFC, ...)]),
					[enable_experimental=${enableval}])
AM_CONDITIONAL(EXPERIMENTAL, test "${enable_experimental}" = "yes")

AC_ARG_ENABLE(deprecated, AC_HELP_STRING([--enable-deprecated],
			[enable deprecated plugins (BLE services, ...)]),
					[enable_deprecated=${enableval}])
AM_CONDITIONAL(DEPRECATED, test "${enable_deprecated}" = "yes")

AC_ARG_ENABLE(sixaxis, AC_HELP_STRING([--enable-sixaxis],
		[enable sixaxis plugin]), [enable_sixaxis=${enableval}])
AM_CONDITIONAL(SIXAXIS, test "${enable_sixaxis}" = "yes" &&
					 test "${enable_udev}" != "no")

AC_ARG_ENABLE(extaqis, AC_HELP_STRING([--enable-extaqis],
			[enable extaqis plugins (SAP, NFC, ...)]),
					[enable_extaqis=${enableval}])
AM_CONDITIONAL(EXTAQIS, test "${enable_extaqis}" = "yes")

if (test "${prefix}" = "NONE"); then
	dnl no prefix and no localstatedir, so default to /var
	if (test "$localstatedir" = '${prefix}/var'); then
		AC_SUBST([localstatedir], ['/var'])
	fi

	prefix="${ac_default_prefix}"
fi

if (test "$localstatedir" = '${prefix}/var'); then
	storagedir="${prefix}/var/lib/bluetooth"
else
	storagedir="${localstatedir}/lib/bluetooth"
fi
AC_DEFINE_UNQUOTED(STORAGEDIR, "${storagedir}",
			[Directory for the storage files])

if (test "$sysconfdir" = '${prefix}/etc'); then
	configdir="${prefix}/etc/bluetooth"
else
	configdir="${sysconfdir}/bluetooth"
fi
AC_DEFINE_UNQUOTED(CONFIGDIR, "${configdir}",
			[Directory for the configuration files])
AC_SUBST(CONFIGDIR, "${configdir}")

AC_ARG_ENABLE(android, AC_HELP_STRING([--enable-android],
			[enable BlueZ for Android]),
					[enable_android=${enableval}])
AM_CONDITIONAL(ANDROID, test "${enable_android}" = "yes")

if (test "${enable_android}" = "yes"); then
	PKG_CHECK_MODULES(SBC, sbc >= 1.2, dummy=yes,
					AC_MSG_ERROR(SBC library >= 1.2 is required))
	AC_SUBST(SBC_CFLAGS)
	AC_SUBST(SBC_LIBS)
fi

if (test "${enable_android}" = "yes"); then
	PKG_CHECK_MODULES(SPEEXDSP, speexdsp >= 1.2, dummy=yes,
					AC_MSG_ERROR(SPEEXDSP library >= 1.2 is required))
	AC_SUBST(SPEEXDSP_CFLAGS)
	AC_SUBST(SPEEXDSP_LIBS)
fi

AC_DEFINE_UNQUOTED(ANDROID_STORAGEDIR, "${storagedir}/android",
			[Directory for the Android daemon storage files])

AC_OUTPUT(Makefile src/bluetoothd.8 lib/bluez.pc)
