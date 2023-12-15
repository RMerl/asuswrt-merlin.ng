
dnl $Id$

AC_PREREQ([2.69])

dnl Set version info here!
AC_INIT([tcpreplay],[4.4.4],[https://github.com/appneta/tcpreplay/issues],[tcpreplay],[http://tcpreplay.sourceforge.net/])
AC_CONFIG_SRCDIR([src/tcpreplay.c])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_AUX_DIR(config)
AM_MAINTAINER_MODE([enable])
AM_WITH_DMALLOC

dnl People building from GitHub need the same version of Autogen as I'm using
dnl or specify --disable-local-libopts to force using the locally-installed
dnl copy of libopts rather than the source in the `./libopts/` directory.
MAINTAINER_AUTOGEN_VERSION=5.18.16

AC_CONFIG_MACRO_DIR([m4])

TCPREPLAY_VERSION=$PACKAGE_VERSION

dnl Release is only used for the RPM spec file
TCPREPLAY_RELEASE=1

AC_SUBST(TCPREPLAY_VERSION)
AC_SUBST(TCPREPLAY_RELEASE)

dnl Determine OS
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AC_SUBST(host)
AC_SUBST(build)
AC_SUBST(target)

CROSS_ARCH=
CROSS_LD=
build_arch=$(echo $MACHTYPE | cut -d'-' -f1)
if test "$cross_compiling" = "yes" ; then
    CROSS_ARCH="ARCH=$host_cpu"
    CROSS_LD="LD=$LD"
fi
AC_SUBST(CROSS_ARCH)
AC_SUBST(CROSS_LD)


dnl ===========================
dnl Check OS X SDK and compiler
dnl ===========================
AC_ARG_WITH([macos-sdk],
            [AS_HELP_STRING([--with-macos-sdk=VER],
                            [Specify the macOS SDK version to use.])]
)

case "$host_os" in
    darwin*) # Mac OS X or iOS
        # If no --with-macos-sdk option is given, look for the latestq SDK

        # The intent is that for "most" Mac-based developers, a suitable
        # SDK will be found automatically without any configure options.

        # For developers with a current Xcode, the lowest-numbered SDK
        # higher than or equal to the minimum required should be found.

        # To find a list of available version run `xcodebuild -showsdks`

        MULTIARCH=${host_cpu}-${host_os}
        unset MACOSX_SDK_PATH

        AC_MSG_CHECKING([what macOS compiler to use])

        if test -n "$with_macos_sdk"; then
            MACOSX_SDK_PATH=$(xcrun --sdk macosx${with_macos_sdk} --show-sdk-path 2> /dev/null)
            if test -z "$MACOSX_SDK_PATH" ; then
                AC_MSG_ERROR([could not find SDK ${with_macos_sdk} for macos-${MULTIARCH}])
            fi
        else
            MACOSX_SDK_PATH=$(xcrun --show-sdk-path 2> /dev/null)
            if test -z "$MACOSX_SDK_PATH" ; then
                dnl could not find a default SDK - search likely SDK versions
                for _macos_sdk_major in $(seq 15 -1 10); do
                    for _macos_sdk_minor in $(seq 20 -1 0); do
                        _macos_sdk_version=$_macos_sdk_major.$_macos_sdk_minor
                        MACOSX_SDK_PATH=$(xcrun --sdk macosx${_macos_sdk_version} --show-sdk-path 2> /dev/null)
                        if test -d "$_macos_sdk_path" ; then
                            break 2
                        fi
                    done
                done

                dnl could not find a default SDK - search disk for the latest SDK
                if test -z "$MACOSX_SDK_PATH"; then
                    for _macos_sdk_major in $(seq 15 -1 10); do
                        for _macos_sdk_minor in $(seq 20 -1 0); do
                            _macos_sdk_version=$_macos_sdk_major.$_macos_sdk_minor
                            MACOSX_SDK_PATH=$(xcode-select -print-path)/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${_macos_sdk_version}.sdk
                            if test -d "$MACOSX_SDK_PATH"; then
                                break 2
                            fi

                            MACOSX_SDK_PATH=$(xcode-select -print-path)/SDKs/MacOSX${_macos_sdk_version}.sdk
                            if test -d "$MACOSX_SDK_PATH"; then
                                break 2
                            fi
                        done
                    done
                fi
            fi
        fi

        if test -d "$MACOSX_SDK_PATH"; then
            CC="$(xcrun -find clang) -m64 -isysroot $MACOSX_SDK_PATH"
            INSTALL_NAME_TOOL=$(xcrun -find install_name_tool)
            AR=$(xcrun -find ar)
            NM=$(xcrun -find nm)
            STRIP=$(xcrun -find strip)
            LIBTOOL=$(xcrun -find libtool)
            RANLIB=$(xcrun -find ranlib)
            AC_MSG_RESULT([${MACOSX_SDK_PATH}])
        else
            AC_MSG_RESULT([legacy])
        fi
        ;;
    Linux*)
        dnl ============================================
        dnl Some libraries are in /usr/lib/i386-linux-gnu
        dnl but host is i686-linux-gnu
        dnl ============================================
        MULTIARCH=$(dpkg-architecture -qDEB_HOST_MULTIARCH) 2> /dev/null || ${host_cpu}-${host_os}
        ;;
    *)
        MULTIARCH=${host_cpu}-${host_os}
        ;;
esac

AC_SUBST(MACOSX_SDK_PATH)
AC_SUBST(LIBTOOL) # Note that the OS X libtool command is unrelated to GNU libtool

dnl ============================================
dnl PF_RING library option
dnl ============================================

AC_ARG_WITH(pfring-lib,
    AS_HELP_STRING([--with-pfring-lib],
        [Use a specific PF_RING static library when using PF_RING libpcap.])
    [
                          Usage:     --with-pfring-lib=<library>
                          e. g.: --with-pfring-lib=/usr/lib/libpfring_zc_x86_64.a
    ],
,)


have_cygwin=no
AC_MSG_CHECKING([for cygwin development environment])
if test $(echo $target | grep -c cygwin) -gt 0 ; then
    AC_DEFINE([HAVE_WIN32], [1], [Windows/Cygwin])
    AC_MSG_RESULT(yes)
    have_cygwin=yes
else
    AC_MSG_RESULT(no)
fi


AM_INIT_AUTOMAKE([1.15 foreign subdir-objects -Wall -Wno-override])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Checks for programs.
AM_PROG_AR
AC_PROG_INSTALL
LT_INIT
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_LN_S
LT_INIT
AC_PROG_AWK
AC_PROG_SED
AC_PROG_MAKE_SET
AC_EXEEXT
AC_PATH_PROG(PRINTF, printf)
AC_PATH_PROG(ECHO, echo)
AC_PATH_PROG(CUT, cut)
AC_PATH_PROG(AUTOGEN, autogen)
AC_PATH_PROG(GROFF, groff)
AC_PATH_PROG(depmod, depmod, /sbin/depmod, $PATH:/sbin)
AC_PATH_PROG(insmod, insmod, /sbin/insmod, $PATH:/sbin)
AC_PATH_PROG(rmmod, rmmod, /sbin/rmmod, $PATH:/sbin)


dnl tcpreplay has (so far) been relying on leading-edge autogen.
dnl Therefore, by default:
dnl - use the version we ship with
dnl - do not install it
dnl - build a static copy (AC_DISABLE_SHARED - implicitly done earlier)
case "${enable_local_libopts+set}" in
 set) ;;
 *) enable_local_libopts=yes ;;
esac

case "${enable_libopts_install+set}" in
 set) ;;
 *) enable_libopts_install=no ;;
esac

dnl check autogen version
AUTOGEN_VERSION="unknown - man pages will not be built"
if test -n "${AUTOGEN}" ; then
    AC_MSG_CHECKING(for autogen version >= ${MAINTAINER_AUTOGEN_VERSION})
    AUTOGEN_VERSION=$(${AUTOGEN} -v | ${CUT} -d' ' -f 4)
    AUTOGEN_MAJOR=$(echo ${AUTOGEN_VERSION} | ${CUT} -d '.' -f 1)
    AUTOGEN_MINOR=$(echo ${AUTOGEN_VERSION} | ${CUT} -d '.' -f 2)
    AUTOGEN_BUILD=$(echo ${AUTOGEN_VERSION} | ${CUT} -d '.' -f 3)
    if test -z "$AUTOGEN_BUILD"; then
        AUTOGEN_BUILD=0
    fi
    if (test ${AUTOGEN_MAJOR} -eq 5 && test ${AUTOGEN_MINOR} -eq 18 && test ${AUTOGEN_BUILD} -lt 4) ||
            (test ${AUTOGEN_MAJOR} -eq 5 && test ${AUTOGEN_MINOR} -lt 18) ||
            test ${AUTOGEN_MAJOR} -lt 5 ; then
        AC_MSG_RESULT(no)
        if test ! "x$enable_local_libopts" = "xyes"; then
            AC_MSG_ERROR([${AUTOGEN} is too old (${AUTOGEN_VERSION}) for building from source code. Upgrade to 5.18.4 or higher])
        fi
        AUTOGEN_VERSION="${AUTOGEN_VERSION} - downlevel"
    else
        AC_MSG_RESULT(yes)
    fi

    dnl Compare the installed version with the maintainer version if building from GitHub and not using system libopts
    if test ! -f src/tcpreplay_opts.c && test "x$enable_local_libopts" = "xyes" ; then
        if test $MAINTAINER_AUTOGEN_VERSION != $AUTOGEN_VERSION ; then
            AC_MSG_ERROR([Your version of autogen ($AUTOGEN_VERSION) != libopts tear off ($MAINTAINER_AUTOGEN_VERSION)  Either install the correct version or specify --disable-local-libopts])
        fi
    fi
else
    if test ! -f src/tcpreplay_opts.c ; then
        AC_MSG_ERROR([Please install GNU autogen $MAINTAINER_AUTOGEN_VERSION or higher if you are building from GitHub. To avoid this message download source from https://github.com/appneta/tcpreplay/releases/latest])
    fi
fi
AC_DEFINE([AUTOGEN_VERSION], [${AUTOGEN_VERSION}], [What version of autogen is installed on this system])

if test "x$enable_local_libopts" = "xyes" ; then
    AC_MSG_NOTICE([Using included libopts tearoff])
else
    AC_MSG_NOTICE([Using system libopts])
fi

AC_PROG_EGREP
AC_HEADER_MAJOR

dnl Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_HEADERS_ONCE([sys/time.h])
AC_C_CONST
AC_C_INLINE
AC_SYS_LARGEFILE

dnl Check for functions
AC_FUNC_FSEEKO

dnl Check for types.
AC_CHECK_TYPE(u_int8_t, uint8_t)
AC_CHECK_TYPE(u_int16_t, uint16_t)
AC_CHECK_TYPE(u_int32_t, uint32_t)
AC_CHECK_TYPE(u_int64_t, uint64_t)

dnl OS X SDK 10.11 throws lots of unnecessary macro warnings
wno_format=""
wno_macro_redefined=""
case $host in
    *-apple-darwin*)
        AC_MSG_CHECKING(for $CC -Wno-macro-redefined)
        OLD_CFLAGS=$CFLAGS
        CFLAGS="$CFLAGS -Wno-macro-redefined -Werror"
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdlib.h>
            int main(int argc, char *argv[]) { return(0); }]])],
            [ AC_MSG_RESULT(yes)
              wno_macro_redefined="-Wno-macro-redefined" ],
            [ AC_MSG_RESULT(no) ])
        AC_MSG_CHECKING(for $CC -Wno-format)
        CFLAGS="$OLD_CFLAGS $wno_macro_redefined"
        CFLAGS="$CFLAGS -Wno-format -Werror"
        AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdlib.h>
            int main(int argc, char *argv[]) { return(0); }]])],
        [ AC_MSG_RESULT(yes)
          wno_format="-Wno-format" ],
        [ AC_MSG_RESULT(no) ])
        CFLAGS="$OLD_CFLAGS $wno_format"
    ;;
esac

dnl Older versions of GCC don't support these options
AC_MSG_CHECKING(for $CC -Wextra support)
OLD_CFLAGS=$CFLAGS
CFLAGS="$CFLAGS -Wextra -Werror"
wextra=""
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdlib.h>
    int main(int argc, char *argv[]) { return(0); }]])],
    [ AC_MSG_RESULT(yes)
      wextra="-Wextra" ],
    [ AC_MSG_RESULT(no) ])
CFLAGS="$OLD_CFLAGS $wextra"

AC_MSG_CHECKING(for $CC -Wno-variadic-macros support)
OLD_CFLAGS=$CFLAGS
CFLAGS="$CFLAGS -Wno-variadic-macros -Werror"
wno_variadic_macros=""
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdlib.h>
    int main(int argc, char *argv[]) { return(0); }]])],
    [ AC_MSG_RESULT(yes)
      wno_variadic_macros="-Wno-variadic-macros" ],
    [ AC_MSG_RESULT(no) ])
CFLAGS="$OLD_CFLAGS $wno_variadic_macros"

AC_MSG_CHECKING(for $CC -Wfatal-errors support)
OLD_CFLAGS=$CFLAGS
CFLAGS="$CFLAGS -Wfatal-errors -Werror"
wfatal_errors=""
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdlib.h>
    int main(int argc, char *argv[]) { return(0); }]])],
    [ AC_MSG_RESULT(yes)
      wfatal_errors="-Wfatal-errors" ],
    [ AC_MSG_RESULT(no) ])
CFLAGS="$OLD_CFLAGS $wfatal_errors"

AC_MSG_CHECKING(for $CC -Wno-format-contains-nul support)
OLD_CFLAGS=$CFLAGS
CFLAGS="$CFLAGS -Wno-format-contains-nul -Werror"
wno_format_contains_nul=""
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[#include <stdlib.h>
    int main(int argc, char *argv[]) { return(0); }]])],
    [ AC_MSG_RESULT(yes)
      wno_format_contains_nul="-Wno-format-contains-nul" ],
    [ AC_MSG_RESULT(no) ])
CFLAGS="$OLD_CFLAGS $wno_format_contains_nul"

dnl Check for other header files
AC_CHECK_HEADERS([fcntl.h stddef.h sys/socket.h  arpa/inet.h sys/time.h])
AC_CHECK_HEADERS([signal.h string.h strings.h sys/types.h stdint.h sys/select.h])
AC_CHECK_HEADERS([netinet/in.h netinet/in_systm.h poll.h sys/poll.h unistd.h sys/param.h])
AC_CHECK_HEADERS([inttypes.h libintl.h sys/file.h sys/ioctl.h sys/systeminfo.h])
AC_CHECK_HEADERS([sys/io.h architecture/i386/pio.h sched.h fts.h])
AC_HEADER_STDBOOL

dnl OpenBSD has special requirements
AC_CHECK_HEADERS([sys/sysctl.h net/route.h], [], [], [
[#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
]])


dnl Checks for libraries.
AC_CHECK_LIB(socket, socket)
AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(rt, nanosleep)
AC_CHECK_LIB(resolv, resolv)

dnl Checks for library functions.
AC_FUNC_FORK
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MEMCMP
AC_FUNC_VPRINTF
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_CHECK_MEMBERS([struct timeval.tv_sec])
AC_CHECK_FUNCS([alarm atexit bzero dup2 gethostbyname getpagesize gettimeofday])
AC_CHECK_FUNCS([ctime inet_ntoa memmove memset munmap pow putenv realpath])
AC_CHECK_FUNCS([regcomp strdup select socket strcasecmp strchr strcspn strdup])
AC_CHECK_FUNCS([strerror strtol strncpy strtoull poll ntohll mmap snprintf])
AC_CHECK_FUNCS([vsnprintf strsignal strpbrk strrchr strspn strstr strtoul])
AC_CHECK_FUNCS([ioperm])

dnl Look for strlcpy since some BSD's have it
AC_CHECK_FUNCS([strlcpy],have_strlcpy=true,have_strlcpy=false)
AM_CONDITIONAL(SYSTEM_STRLCPY, [test x$have_strlcpy = xtrue])

AC_C_BIGENDIAN
AM_CONDITIONAL([WORDS_BIGENDIAN], [ test x$ac_cv_c_bigendian = xyes ])

AC_ARG_ENABLE([asan],
    AS_HELP_STRING([--enable-asan],[Enable Address Sanitizer support]))
if test "x$enable_asan" = "xyes"; then
        build_asan=yes
        AC_CHECK_LIB([asan], [__asan_report_error], ,
                [AC_MSG_ERROR([libasan.so not found, this is required for --enable-asan])])
        CFLAGS="${CFLAGS} -O1 -g -fsanitize=address -fno-omit-frame-pointer"
        dnl -lasan always need to be the first library
        LDFLAGS="-fsanitize=address ${LDFLAGS}"
else
        build_asan=no
fi
AC_SUBST(build_asan)

AC_ARG_ENABLE([tsan],
    AS_HELP_STRING([--enable-tsan],[Enable Thread Sanitizer support]))
if test "x$enable_tsan" = "xyes"; then
        build_tsan=yes
        CFLAGS="${CFLAGS} -O1 -g -fsanitize=thread -fno-omit-frame-pointer"
        dnl -ltsan always need to be the first library
        LDFLAGS="-fsanitize=thread ${LDFLAGS}"
else
        build_tsan=no
fi
AC_SUBST(build_tsan)

if test "$build_asan" = "yes" -a "$build_tsan" = "yes" ; then
    AC_MSG_ERROR([--enable-asan and --eanble-tsan are mutually exclusive])
fi

dnl Enable debugging in code/compiler options
debug=no
debug_flag=NDEBUG
debug_run_time_flag=
AC_ARG_ENABLE(debug,
    AS_HELP_STRING([--enable-debug],[Enable debugging code and support for the -d option]),
    [ if test x$enableval = xyes; then
        debug=yes
        dnl replace '-0X' and add '-g' flags
        changequote({,})
        CFLAGS=`echo "$CFLAGS" | $SED -e 's/-O[0-9s]*//g'`
        changequote([,])
        CFLAGS="${CFLAGS} -g -O0 -Wall $wextra $wfatal_errors $wno_variadic_macros $wno_format_contains_nul $wno_format"
# We may also want to add:
#         -Wformat-security -Wswitch-default -Wunused-paramter -Wpadded"
        debug_flag=DEBUG
        debug_run_time_flag=--dbug=1
        AC_DEFINE([DEBUG], [1], [Enable debugging code and support for the -d option])
    fi])
AC_SUBST(debug_flag)
AC_SUBST(debug_run_time_flag)

dnl Enable extra debugging in code/compiler options - may slow down performance
extra_debug=no
extra_debug_flag=NEXTRA_DBUG
AC_ARG_ENABLE(extra-debug,
    AS_HELP_STRING([--enable-extra-debug],[Enable additional debugging code (may affect performance)]),
    [ if test x$enableval = xyes; then
        extra_debug=yes
        extra_debug_flag=EXTRA_DEBUG
        AC_DEFINE([EXTRA_DEBUG], [1], [Enable additional debugging code (may affect performance)])
    fi])
AC_SUBST(extra_debug_flag)

dnl Enable timestamp_trace in code/compiler options
timestamp_trace=no
AC_ARG_ENABLE(timestamp-trace,
    AS_HELP_STRING([--timestamp-trace],[Enable dumping of trace timestamps at the end of a test]),
    [ if test x$enableval = xyes; then
        timestamp_trace=yes
        CFLAGS="${CFLAGS} -DTIMESTAMP_TRACE"
        AC_DEFINE([TIMESTAMP_TRACE], [1], [Enable dumping of trace timestamps at the end of a test])
    fi])
AC_SUBST(timestamp_trace_flag)


AC_ARG_ENABLE(dmalloc,
    AS_HELP_STRING([--enable-dmalloc],[Enable linking to dmalloc for better memory debugging]),
    [ if test x$enableval = xyes ; then
        dmalloc=yes
        AC_DEFINE([ENABLE_DMALLOC], [1], [Enable dmalloc])
        AC_DEFINE([DMALLOC_FUNC_CHECK], [1], [Enable dmalloc function arg checking])
        CFLAGS="${CFLAGS} -I/opt/local/include"
        DMALLOC_LIB="/opt/local/lib/libdmalloc.a"
        AC_SUBST(DMALLOC_LIB)
    fi])


AC_ARG_ENABLE(pedantic,
    AS_HELP_STRING([--enable-pedantic],[Enable gcc's -pedantic option]),
    [ if test x$enableval = xyes; then
        CFLAGS="$CFLAGS -pedantic"
      fi ])

dnl Enable Efense
AC_ARG_ENABLE(efence,
    AS_HELP_STRING([--enable-efence],[Enable Electric Fence memory debugger]),
    [ if test x$enableval = xyes; then
          CFLAGS="$CFLAGS -lefence"
          AC_DEFINE([EFENCE], [1], [Enable Electric Fence memory debugger])
      fi])

dnl Enable Gprof
AC_ARG_ENABLE(gprof,
    AS_HELP_STRING([--enable-gprof],[Enable GNU Profiler]),
    [ if test x$enableval = xyes; then
          if test $debug = yes; then
              CFLAGS="$CFLAGS -pg"
          else
              # GPROF requires gdb
              CFLAGS="$CFLAGS -ggdb -pg -fprofile-arcs"
          fi
          AC_DEFINE([GPROF], [1], [Enable GNU Profiler])
      fi])

dnl Enable libpcap auto discovery via pcap-config
enable_pcap_config=no
AC_ARG_ENABLE(pcapconfig,
    AS_HELP_STRING([--enable-pcapconfig],[Enable automatic libpcap config, reverting to system scanning]),
    [
        if test $enableval = yes; then
            enable_pcap_config=yes
            AC_MSG_NOTICE([Enabling libpcap discovery via pcap_config])
        else
            enable_pcap_config=no
            AC_MSG_NOTICE([Disabling libpcap discovery via pcap_config])
        fi
    ],
    [
        AC_MSG_NOTICE([Bypassing libpcap discovery via pcap_config])
    ])

dnl Use 64bits for packet counters
use64bit_counters=yes
AC_ARG_ENABLE(64bits,
    AS_HELP_STRING([--disable-64bits],[Do not use 64bit packet counters]),
    [
         AC_MSG_NOTICE([Using u_int32_t for packet counters])
    ],
    [
         AC_DEFINE([ENABLE_64BITS], [1], [Use 64bit packet counters])
         AC_MSG_NOTICE([Using u_int64_t for packet counters])
    ])


# If we're running gcc add '-D_U_="__attribute__((unused))"' to CFLAGS as well,
# so we can use _U_ to flag unused function arguments and not get warnings
# about them. Otherwise, add '-D_U_=""', so that _U_ used to flag an unused
# function argument will compile with non-GCC compilers.
#
if test "x$GCC" = "xyes" ; then
    CFLAGS="-D_U_=\"__attribute__((unused))\" $CFLAGS"
else
    CFLAGS="-D_U_=\"\" $CFLAGS"
fi


AC_ARG_ENABLE(force-bpf,
    AS_HELP_STRING([--enable-force-bpf],[Force using BPF for sending packets]),
    [ AC_DEFINE([FORCE_INJECT_BPF], [1], [Force using BPF for sending packet])])

AC_ARG_ENABLE(force-pf,
    AS_HELP_STRING([--enable-force-pf],[Force using Linux's PF_PACKET for sending packets]),
    [ AC_DEFINE([FORCE_INJECT_PF], [1], [Force using Linux's PF_PACKET for sending packets])])

AC_ARG_ENABLE(force-libdnet,
    AS_HELP_STRING([--enable-force-libdnet],[Force using libdnet for sending packets]),
    [ AC_DEFINE([FORCE_INJECT_LIBDNET], [1], [Force using libdnet for sending packets])])

AC_ARG_ENABLE(force-inject,
    AS_HELP_STRING([--enable-force-inject],[Force using libpcap's pcap_inject() for sending packets]),
    [ AC_DEFINE([FORCE_INJECT_PCAP_INJECT],[1], [Force using libpcap's pcap_inject() for sending packets])])

AC_ARG_ENABLE(force-sendpacket,
    AS_HELP_STRING([--enable-force-sendpacket],[Force using libpcap's pcap_sendpacket() for sending packets]),
    [ AC_DEFINE([FORCE_INJECT_PCAP_SENDPACKET], [1], [Force using libpcap's pcap_sendpacket() for sending packets])])

dnl Static link libraries
static_link=no
dynamic_link=yes
AC_ARG_ENABLE(static-link,
    AS_HELP_STRING([--enable-static-link],[Use static libraries ( .a  or .A.tbd ) - default no]),
    [
    if test $enableval = yes; then
        static_link=yes
        dynamic_link=no
    fi
    ])

dnl Dynamic link libraries
AC_ARG_ENABLE(dynamic-link,
    AS_HELP_STRING([--enable-dynamic-link],[Use shared libraries ( .so .dylib or .tbd ) - default yes]),
    [
    if test $enableval = no; then
        dynamic_link=no
        static_link=yes
    elif test $static_link = yes; then
        AC_MSG_ERROR([Cannot specify both --enable-dynamic-link and --enable-static-link])
    fi
    ])

if test $static_link = yes; then
    AC_DEFINE([ENABLE_STATIC_LINK], [1], [Use static libraries ( .a or .A.tbd )])
else
    AC_DEFINE([ENABLE_DYNAMIC_LINK], [1], [Use shared libraries ( .so .dylib or .tbd )])
fi

dnl Check for inet_aton and inet_pton
AC_CHECK_FUNC(inet_aton,
    AC_DEFINE([HAVE_INET_ATON], [1], [Do we have inet_aton?])
    inet_aton=yes,
    inet_aton=no)
AC_CHECK_FUNC(inet_pton,
    AC_DEFINE([HAVE_INET_PTON], [1], [Do we have inet_pton?])
    inet_pton=yes,
    inet_pton=no)
AC_CHECK_FUNC(inet_ntop,
    AC_DEFINE([HAVE_INET_NTOP], [1], [Do we have inet_ntop?])
    inet_ntop=yes,
    inet_ntop=no)

if test "$inet_ntop" = "no" -a "$inet_pton" = "no" ; then
    AC_MSG_ERROR([We need either inet_ntop or inet_pton])
fi

AC_CHECK_FUNC(inet_addr,
    AC_DEFINE([HAVE_INET_ADDR], [1], [Do we have inet_addr?])
    inet_addr=yes,
    inet_addr=no)

if test x$inet_addr = no ; then
    AC_MSG_ERROR([We need inet_addr.  See bug 26])
fi

dnl #####################################################
dnl Checks for tuntap device support
dnl #####################################################
have_tuntap=no
AC_ARG_ENABLE([tuntap],
  AS_HELP_STRING([--disable-tuntap], [Disable tuntap support]), [:],
  [case "$build_os" in
    linux*)
      AC_CHECK_HEADER([linux/if_tun.h], [have_tuntap=yes])
      ;;
    *)
      AC_CHECK_HEADER([net/if_tun.h], [have_tuntap=yes])
      ;;
  esac])

if test $have_tuntap = yes ; then
    AC_DEFINE([HAVE_TUNTAP], [1],
            [Do we have TUNTAP device support?])
fi

dnl #####################################################
dnl Checks for libpcap
dnl #####################################################
foundpcap=no
pf_ring_found=no
pf_ring_lib=${with_pfring_lib}
using_pcap_config=no
AC_MSG_CHECKING(for libpcap)
AC_ARG_WITH(libpcap,
    AS_HELP_STRING([--with-libpcap=DIR],[Use libpcap in DIR]),
[
    testdir=$withval
    for incl in ${testdir}/include/pcap.h ${testdir}/pcap.h ${testdir}/pcap/pcap.h; do
        if ! test -f "${incl}"; then
            continue
        fi
        LPCAPINC=${incl}
        LPCAPINCDIR=${testdir}
        if test $dynamic_link = yes; then
            for ext in .dylib .so .tbd ; do
                for dir in . lib lib64 lib/${host_cpu}-${host_os} ${host_cpu}-${host_os} lib/${MULTIARCH} ${MULTIARCH}; do
                    sharefile=$(ls ${testdir}/$dir/libpcap${ext}* 2> /dev/null | sort | head -n1)
                    if test -n "${sharefile}"; then
                        LPCAP_LD_LIBRARY_PATH="$(dirname ${sharefile})"
                        LPCAPLIB="-L$LPCAP_LD_LIBRARY_PATH -lpcap"
                        foundpcap=$testdir
                        break
                    fi
                done

                if ! test $foundpcap = no; then
                    break
                fi
            done
        else
            dnl
            dnl If dynamic library not found, try static
            dnl
            for ext in ${libext} .a .A.tbd ; do
                for dir in . lib lib64 lib/${host_cpu}-${host_os} ${host_cpu}-${host_os} lib/${MULTIARCH} ${MULTIARCH}; do
                    staticfile=$(ls ${testdir}/$dir/libpcap${ext} 2> /dev/null | sort | head -n1)
                    if test -n "${staticfile}"; then
                        LPCAPLIB="${staticfile}"
                        foundpcap=${testdir}
                        break
                    fi
                done

            done
        fi

        if test $foundpcap = no; then
            AC_MSG_ERROR("Unable to find matching library for header file in ${testdir}")
        fi

        break
    done
],[
    #
    # Look for pcap-config.
    #
    # I wish this option could be the default, but it is only enabled with --enable-pcapconfig
    #
    # The issue is that libpcap is one of those libraries that is nearly impossible to set up
    # properly across all platforms. The 'pcap-config' script is shipped with new versions of libpcap,
    # and was created to address portability issues. Unfortunately, the script is not updated properly
    # outside of the base distribution. Examples: PF_RING requires 'pthreads', 'numa' and 'rt', but
    # this is not reflect that in the script. OS X Xcode 10.11 SDK creates its own very broken
    # 'pcap-config' script that only occasionally works for dynamic definitions, and definitely
    # does not work for static.
    #
    # The ability to enable this script exists for corner cases where libpcap distributions starts
    # using even more libraries than I am currently not aware of. Originally, you only had to worry
    # about 'nl', but now we have to be prepared to add 'nl-3', 'nl-genl-3' and 'dbus-1' to make
    # libpcap functional. In the future, there may be more.
    #
    # (sigh)
    #
    # So, in the future if libpcap changes and stops working, try setting --enable-pcapconfig. It
    # it may work for you.
    if test $enable_pcap_config = yes; then
        AC_PATH_TOOL(PCAP_CONFIG, pcap-config)
            if test -n "$PCAP_CONFIG"; then
            #
            # Found - use it to get the include flags for
            # libpcap and the flags to link with libpcap.
            #
            # Please read section 11.6 "Shell Substitutions"
            # in the autoconf manual before doing anything
            # to this that involves quoting.  Especially note
            # the statement "There is just no portable way to use
            # double-quoted strings inside double-quoted back-quoted
            # expressions (pfew!)."
            #

            if test $dynamic_link = yes; then
                LPCAPLIB=$($PCAP_CONFIG --libs)
            else
                LPCAPLIB=$($PCAP_CONFIG --libs --static)
            fi
            CFLAGS="$CFLAGS $($PCAP_CONFIG --cflags)"
            if test -n "$LPCAPLIB"; then
                foundpcap=yes
                using_pcap_config=yes
            else
                AC_MSG_WARN(Unable to find libpcap using pcap-config)
            fi
        fi
    fi

    if test ! $foundpcap = yes; then
        #
        # Not found; look for pcap.
        #
        for testdir in /usr/local /opt/local /usr /wpdpack $MACOSX_SDK_PATH/usr /usr/local/opt/libpcap ; do
            if test -f "${testdir}/include/pcap.h"; then
                LPCAPINC="${testdir}/include/pcap.h"
                LPCAPINCDIR="${testdir}/include"
                if test $dynamic_link = yes; then
                    for ext in .dylib .so .tbd; do
                        for dir in . lib lib64 ${host_cpu} lib/${host_cpu} ${host_cpu}-${host_os} lib/${host_cpu}-${host_os} ${MULTIARCH} lib/${MULTIARCH}; do
                            sharefile=$(ls "${testdir}/$dir/libpcap${ext}" 2> /dev/null | sort | head -n1)
                            if test -n "${sharefile}"; then
                                LPCAPLIB="-L$(dirname ${sharefile}) -lpcap"
                                foundpcap=$testdir
                                break
                            fi
                        done

                        if ! test $foundpcap = no; then
                            break
                        fi
                    done
                fi
                if test $foundpcap = no ; then
                    dnl
                    dnl If dynamic library not found, try static
                    dnl
                    for ext in ${libext} .a .A.tbd ; do
                        for dir in . lib lib64 lib/${host_cpu}-${host_os} ${host_cpu}-${host_os} lib/${MULTIARCH} ${MULTIARCH}; do
                            staticfile=$(ls "${testdir}/$dir/libpcap${ext}" 2> /dev/null | sort | head -n1)
                            if test -n "${staticfile}"; then
                                LPCAPLIB="${staticfile}"
                                foundpcap=$testdir
                                break
                            fi
                        done

                        if ! test $foundpcap = no; then
                            break
                        fi
                    done
                fi
                if test $foundpcap = no -a -f "${testdir}/lib/libwpcap.${libext}" ; then
                    LPCAPLIB="${testdir}/lib/libwpcap.${libext}"
                    AC_DEFINE([HAVE_WINPCAP], [1], [Do we have WinPcap?])
                    foundpcap=$testdir
                fi
            fi

            if ! test $foundpcap = no; then
                break
            fi
        done
    fi
    ])

if test $foundpcap = no ; then
    AC_MSG_RESULT(no)
    AC_MSG_ERROR(libpcap not found)
else
    AC_MSG_RESULT($foundpcap)
fi

## could be PF_RING
#
#  library is static and in PF_RING build tree
#
#  Need access to libpfring library
#  and must find an associated libpfring.a
if test -n "${with_pfring_lib}"; then
    if test -f ${with_pfring_lib}; then
        pf_ring_lib=${with_pfring_lib}
        pf_ring_found=yes
    else
        AC_MSG_ERROR(["Unable to find PF_RING library ${with_pfring_lib}"])
    fi
fi

# libpcap can require libnl
AC_SEARCH_LIBS([nl_handle_alloc], [nl],
        [AC_MSG_NOTICE([Unable to find nl library - may be needed by libpcap])])

##
## If not automatically configured,
## check for newer and full-featured libpcap's
##
if ! test $using_pcap_config = yes; then
    ## newer libpcap requires dbus and latest nl (e.g. -lnl-genl-3 -lnl-3  -ldbus-1)
    AC_CHECK_LIB(nl, nl_cache_alloc, [nl_found=yes])
    AC_CHECK_LIB(nl-genl-3, genl_connect, [nl_genl_3_found=yes])
    AC_CHECK_LIB(nl-3, nl_cache_alloc, [nl_3_found=yes])
    AC_CHECK_LIB(dbus-1, dbus_malloc, [dbus_1_found=yes])
    if test "$nl_found" = "yes"; then
        LPCAPLIB="$LPCAPLIB -lnl"
    fi
    if test "$nl_genl_3_found" = "yes"; then
        LPCAPLIB="$LPCAPLIB -lnl-genl-3"
    fi
    if test "$nl_3_found" = "yes"; then
        LPCAPLIB="$LPCAPLIB -lnl-3"
    fi
    if test "$dbus_1_found" = "yes"; then
        LPCAPLIB="$LPCAPLIB -ldbus-1"
    fi

    ## See if prereqs for PF_RING exist
    AC_CHECK_LIB(numa, numa_available, [numa_found=yes])
    AC_CHECK_LIB(pthread, pthread_create, [pthread_found=yes])
    if test $pf_ring_found = yes; then
        if ! test "${numa_found}" = "yes"; then
            AC_MSG_ERROR([PF_RING requires libnuma and/or libnuma-dev])
        fi
        if ! test "${pthread_found}" = "yes"; then
            AC_MSG_ERROR([PF_RING requires libpthread])
        fi
        LPCAPLIB="$LPCAPLIB ${pf_ring_lib} -lnuma -lpthread -lrt"
    fi
fi

OLDLIBS="$LIBS"
if test -n "$LPCAPINCDIR"; then
    OLDCFLAGS="$CFLAGS -I$LPCAPINCDIR"
    CFLAGS="$CFLAGS -I$LPCAPINCDIR"
else
    OLDCFLAGS="$CFLAGS"
    LPCAPINCDIR=$(echo $CFLAGS | sed -e 's/^\-I//')
    if test -d "$LPCAPINCDIR/pcap"; then
        LPCAPINC="$LPCAPINCDIR/pcap/pcap.h"
    else
        LPCAPINC="$LPCAPINCDIR/pcap.h"
    fi
fi

LIBS="$LPCAPLIB"
AC_SEARCH_LIBS([pcap_get_pfring_id],
               [pcap],
               AC_DEFINE([HAVE_PF_RING_PCAP],
               ,
               [${with_pfring_lib} numa pthread rt]))

## Does libpcap work with selected libraries?
AC_SEARCH_LIBS([pcap_close], [pcap],
    , AC_MSG_ERROR(Unable to link libpcap in ${foundpcap}),)

AC_SUBST(LPCAPINC)
AC_SUBST(LPCAPLIB)


dnl Check to see what version of libpcap
AC_MSG_CHECKING(libpcap version)
libpcap_version=unknown
libpcap_version_096=no
cat >conftest.c <<EOF
#include <string.h>
#include <stdio.h>
#include "$LPCAPINC"
/*
 * simple proggy to get the version of libpcap
 */
 int main() {
    printf("%s\n", pcap_lib_version());
    return 0;
}
EOF
${CC-cc} -o conftest $CFLAGS $CPPFLAGS $LPCAPLIB \
    conftest.c $LIBS >/dev/null 2>&1
if test -x conftest ; then
    full_libpcap_version=$(LD_LIBRARY_PATH="$LPCAP_LD_LIBRARY_PATH" ./conftest)
    libpcap_version=$(echo "$full_libpcap_version" | ${CUT} -d' ' -f3)
    pcap_version_ok=yes
fi
AC_MSG_RESULT([$libpcap_version])
rm -rf conftest.* core core.conftest 2>&1 > /dev/null

AC_MSG_CHECKING(for valid libpcap version)
if test x$libpcap_version != xunknown; then
    libpcap_major=$(echo ${libpcap_version} | ${CUT} -d '.' -f1)
    libpcap_minor=$(echo ${libpcap_version} | ${CUT} -d '.' -f2)
    libpcap_build=$(echo ${libpcap_version} | ${CUT} -d '.' -f3)
    if test $libpcap_major -eq 0 && \
        (test $libpcap_minor -lt 7 || \
         (test $libpcap_minor -eq 7 && test $libpcap_build -lt 2))
    then
        AC_MSG_ERROR([Libpcap versions < 0.7.2 are not supported
        Please upgrade to version 0.7.2 or better. Your version is $libpcap_version])
    fi

    dnl 0.9.6 (which is still thinks it is 0.9.5 due to a bug) introduces an important
    dnl fix for OSX.  See: http://tcpreplay.synfin.net/trac/ticket/167
    if test $libpcap_major -gt 0 || \
       (test $libpcap_minor -eq 9 && test $libpcap_build -ge 6)
    then
        libpcap_version_096=yes
    fi

    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([unknown])
fi

dnl Check for pcap_setnonblock()
AC_MSG_CHECKING(for pcap_setnonblock)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "$LPCAPINC"
]],[[
    pcap_t *p;
    char *errbuf;

    pcap_setnonblock(p, 1, errbuf);
    exit(0);
]])], [
    have_pcap_setnonblock=yes
    AC_MSG_RESULT(yes)
], [
    have_pcap_setnonblock=no
    AC_MSG_RESULT(no)
])

if test $have_pcap_setnonblock = yes ; then
    AC_DEFINE([HAVE_PCAP_SETNONBLOCK], [1],
              [Does libpcap have pcap_setnonblock?])
fi

dnl Check to see if we've got pcap_datalink_val_to_name()
AC_MSG_CHECKING(for pcap_datalink_val_to_description)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "$LPCAPINC"
]],[[
    if (strcmp(pcap_datalink_val_to_description(1), "Ethernet (10Mb)") == 0)
        exit(0);
    exit(1);
]])],[
    have_dlt_to_desc=yes
    AC_MSG_RESULT(yes)
], [
    have_dlt_to_desc=no
    AC_MSG_RESULT(no)
])

if test $have_dlt_to_desc = yes ; then
    AC_DEFINE([HAVE_DLT_VAL_TO_DESC], [1],
              [Does libpcap have pcap_datalink_val_to_description?])
fi

dnl Check for pcap_get_selectable_fd()
AC_MSG_CHECKING(for pcap_get_selectable_fd)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "$LPCAPINC"
]], [[
    int f;
    pcap_t *p;
    f = pcap_get_selectable_fd(p);
    exit(0);
]])], [
    have_pcap_get_selectable_fd=yes
    AC_MSG_RESULT(yes)
], [
    have_pcap_get_selectable_fd=no
    AC_MSG_RESULT(no)
])

if test x$have_pcap_get_selectable_fd = xyes ; then
    AC_DEFINE([HAVE_PCAP_GET_SELECTABLE_FD], [1],
        [Does libpcap have pcap_get_selectable_fd?])
fi

dnl Important: winpcap apparently defines functions in it's header files
dnl which aren't actually in the library.  Totally fucked up.  Hence, we
dnl must actually LINK the code, not just compile it.

dnl Check to see if we've got pcap_dump_fopen()
AC_MSG_CHECKING(for pcap_dump_fopen)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "$LPCAPINC"
]],[[
    pcap_dumper_t *dump;
    pcap_t *pcap;
    FILE *foo;
    dump = pcap_dump_fopen(pcap, foo);
]])],[
    have_pcap_dump_fopen=yes
    AC_MSG_RESULT(yes)
], [
    have_pcap_dump_fopen=no
    AC_MSG_RESULT(no)
])

if test $have_pcap_dump_fopen = yes ; then
    AC_DEFINE([HAVE_PCAP_DUMP_FOPEN], [1],
              [Does libpcap have pcap_dump_fopen?])
else
    AC_MSG_NOTICE([--verbose mode requires libpcap >= 0.9.0])
fi

have_pcap_inject=no
dnl Check to see if we've got pcap_inject()
AC_MSG_CHECKING(for pcap_inject sending support)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "$LPCAPINC"
]],[[
    pcap_t *pcap;
    char *buf;
    pcap_inject(pcap, (void *)buf, 0);
]])],[
    have_pcap_inject=yes
    AC_MSG_RESULT(yes)
], [
    have_pcap_inject=no
    AC_MSG_RESULT(no)
])

if test $have_pcap_inject = yes ; then
    AC_DEFINE([HAVE_PCAP_INJECT], [1], [Does libpcap have pcap_inject?])
fi

have_pcap_sendpacket=no
dnl Check to see if we've got pcap_sendpacket()
AC_MSG_CHECKING(for pcap_sendpacket sending support)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "$LPCAPINC"
]],[[
    pcap_t *pcap;
    u_char *buf;
    pcap_sendpacket(pcap, buf, 0);
]])],[
    have_pcap_sendpacket=yes
    AC_MSG_RESULT(yes)
], [
    have_pcap_sendpacket=no
    AC_MSG_RESULT(no)
])

if test $have_pcap_sendpacket = yes ; then
    AC_DEFINE([HAVE_PCAP_SENDPACKET], [1], [Does libpcap have pcap_sendpacket?])
fi

have_pcap_breakloop=no
AC_MSG_CHECKING(for pcap_breakloop)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "$LPCAPINC"
]],[[
    pcap_t *pcap;
    pcap_sendpacket(pcap);
]])],[
    have_pcap_breakloop=yes
    AC_MSG_RESULT(yes)
], [
    have_pcap_breakloop=no
    AC_MSG_RESULT(no)
])

if test $have_pcap_breakloop = yes ; then
    AC_DEFINE([HAVE_PCAP_BREAKLOOP], [1], [Does libpcap have pcap_breakloop?])
fi


have_pcap_version=no
dnl Check to see if we have pcap_version[]
AC_MSG_CHECKING(for pcap_version[])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "$LPCAPINC"
extern char pcap_version[];
]], [[
    printf("%s", pcap_version);
]])],[
    AC_DEFINE([HAVE_PCAP_VERSION],[1], [Does libpcap have pcap_version[]])
    AC_MSG_RESULT(yes)
],[
    AC_MSG_RESULT(no)
])

have_pcap_snapshot=no
dnl Check for pcap_snapshot()
AC_MSG_CHECKING(for pcap_snapshot)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "$LPCAPINC"
]],[[
    pcap_t *p;
    int len;

    len = pcap_snapshot(p);
    exit(0);
]])], [
    have_pcap_snapshot=yes
    AC_MSG_RESULT(yes)
], [
    have_pcap_snapshot=no
    AC_MSG_RESULT(no)
])

if test $have_pcap_snapshot = yes ; then
    AC_DEFINE([HAVE_PCAP_SNAPSHOT], [1],
              [Does libpcap have pcap_snapshot?])
fi


# Tcpbridge requires libpcap and pcap_sendpacket()
enable_tcpbridge=no
if test x$foundpcap != xno -a x$have_pcap_sendpacket = xyes ; then

    # if we have pcap_sendpacket() then we can build tcpbridge on non-BPF systems
    if test x$have_bpf = xno; then
        AC_DEFINE(ENABLE_TCPBRIDGE, [1], [Compile tcpbridge])
        enable_tcpbridge=yes
        AC_MSG_NOTICE([Building tcpbridge])

    # BPF systems need to also have pcap_setnonblock()
    elif test x$have_pcap_setnonblock = xyes ; then
        AC_DEFINE(ENABLE_TCPBRIDGE, [1], [Compile tcpbridge])
        enable_tcpbridge=yes
        AC_MSG_NOTICE([Building tcpbridge])
    else
        AC_MSG_WARN([Unable to build tcpbridge.  Try installing latest libpcap!])
    fi
fi
AM_CONDITIONAL(COMPILE_TCPBRIDGE, [test x$enable_tcpbridge = xyes ])



# tcpliveplay requires linux OS to function properly
enable_tcpliveplay=no
case $host in
    *-*-linux*)
    AC_DEFINE(ENABLE_TCPLIVEPLAY, [1], [Compile tcpliveplay])
    enable_tcpliveplay=yes
    AC_MSG_NOTICE([Building tcpliveplay])
    ;;

    *)
    AC_MSG_WARN([Unable to build tcpliveplay.  This tool is only enabled on linux environments.])
    ;;
esac

AM_CONDITIONAL(COMPILE_TCPLIVEPLAY, [test x$enable_tcpliveplay = xyes ])



dnl #####################################################
dnl Check for netmap support
dnl #####################################################
have_netmap=no
enable_netmap=no
trynetmapdir=
AC_MSG_CHECKING(for netmap socket sending support)
AC_ARG_WITH(netmap,
    AS_HELP_STRING([--with-netmap=DIR],[Use netmap in DIR]),
    [trynetmapdir=$withval])

NETMAP_SEARCH_DIRS=
if test "$cross_compiling" = yes; then
    if test "x$trynetmapdir" != "x"; then
        NETMAP_SEARCH_DIRS=$trynetmapdir
    fi
else
    NETMAP_SEARCH_DIRS="$trynetmapdir /opt/netmap /usr/src/netmap-release /usr/src/netmap /usr/local/src/netmap-release /usr/local/src/netmap /usr/include /usr/local/include"
fi

for testdir in $NETMAP_SEARCH_DIRS; do
    if test -f "${testdir}/sys/net/netmap.h" ; then
        NETMAPINC="${testdir}/sys/net/netmap.h"
        NETMAPUSERINC="${testdir}/sys/net/netmap_user.h"
        NETMAPINCDIR="${testdir}/sys"
        have_netmap="yes $testdir"
        enable_netmap=yes
    else
        if test -f "${testdir}/net/netmap.h" ; then
            NETMAPINC="${testdir}/net/netmap.h"
            NETMAPUSERINC="${testdir}/net/netmap_user.h"
            NETMAPINCDIR="${testdir}"
            have_netmap="yes $testdir"
            enable_netmap=yes
        fi
    fi

    if test "$have_netmap" != no ; then
        NETMAPFLAGS="-DHAVE_NETMAP"
        OLDCFLAGS="$CFLAGS -I$NETMAPINCDIR"
        CFLAGS="$CFLAGS -DNETMAP_WITH_LIBS -DND -I$NETMAPINCDIR"

        AC_SUBST(NETMAPINC)
        AC_SUBST(NETMAPUSERINC)
        AC_SUBST(NETMAPINCDIR)
        AC_SUBST(NETMAPFLAGS)
        AC_DEFINE([HAVE_NETMAP], [1],
               [Do we have netmap support?])
        have_netmap="yes $testdir"
        enable_netmap=yes
        break;
    fi
done

AC_MSG_RESULT($have_netmap)
AM_CONDITIONAL(COMPILE_NETMAP, [test x$enable_netmap = xyes ])


dnl ###########################################################
dnl Check for nm_open() function available in netmap version 5
dnl Also check for other version-specific netmap definitions and structures
dnl ###########################################################

if test "x$NETMAPINCDIR" != "x"; then
    OLDCPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS -DNETMAP_WITH_LIBS -DND -I$NETMAPINCDIR"

    have_nm_open=no
    have_nm_nr_reg_mask=no
    have_nm_nr_flags=no
    have_nmring_head_tail=no
    if test "$have_netmap" != no ; then
        dnl check for nm_open()
        AC_CHECK_DECL([nm_open],[have_nm_open=yes], , [[
            #include <stdio.h>
            #include "${NETMAPUSERINC}"
        ]])

        dnl check for #define NR_REG_MASK
        AC_MSG_CHECKING(for netmap NR_REG_MASK definition)
        AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM([[
                    #include <stdint.h>
                    #include <sys/socket.h>
                    #include <net/if.h>
                    #include <net/netmap.h>
                ]],
                [[
                    #ifdef NR_REG_MASK
                        /* OK */
                    #else
                        # error NR_REG_MASK not found
                    #endif
                ]])
            ],
            [have_nm_nr_reg_mask=yes],
            [])
        AC_MSG_RESULT($have_nm_nr_reg_mask)

        dnl check for nr_flags in struct nmreq
        AC_MSG_CHECKING(for netmap nr_flags definition)
        AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM([[
                    #include <stdint.h>
                    #include <sys/socket.h>
                    #include <net/if.h>
                    #include <net/netmap.h>
                ]],
                [[
                    struct nmreq nmr;
                    nmr.nr_flags = 0;
                ]])
            ],
            [have_nm_nr_flags=yes],
            [])
        AC_MSG_RESULT($have_nm_nr_flags)

        dnl check for head/tail in struct netmap_ring
        AC_MSG_CHECKING(for netmap ring head/tail definitions)
        AC_COMPILE_IFELSE(
            [AC_LANG_PROGRAM([[
                    #include <stdint.h>
                    #include <sys/socket.h>
                    #include <net/if.h>
                    #include <net/netmap.h>
                ]],
                [[
                    struct netmap_ring r;
                    r.head = r.tail = 0;
                ]])
            ],
            [have_nmring_head_tail=yes],
            [])
        AC_MSG_RESULT($have_nmring_head_tail)

    fi

    if test "$have_nm_open" = yes ; then
        AC_DEFINE([HAVE_NETMAP_NM_OPEN], [1], [Does netmap have nm_open function?])
    fi
    if test "$have_nm_nr_reg_mask" = yes ; then
        AC_DEFINE([HAVE_NETMAP_NR_REG], [1], [Does netmap have NR_REG_MASK defined?])
    fi
    if test "$have_nm_nr_flags" = yes ; then
        AC_DEFINE([HAVE_NETMAP_NR_FLAGS], [1], [Does netmap struct nmreq have nr_flags defined?])
    fi
    if test "$have_nmring_head_tail" = yes ; then
        AC_DEFINE([HAVE_NETMAP_RING_HEAD_TAIL], [1], [Does structure netmap_ring have head/tail defined?])
    fi

    CPPFLAGS="$OLDCPPFLAGS"
fi

have_pf=no
dnl Check for linux PF_PACKET support
AC_MSG_CHECKING(for PF_PACKET socket sending support)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>     /* the L2 protocols */
#include <netinet/in.h>       /* htons */
]], [[
    int pf_socket;
    pf_socket = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
]])],[
    AC_DEFINE([HAVE_PF_PACKET], [1],
            [Do we have Linux PF_PACKET socket support?])
    AC_MSG_RESULT(yes)
    have_pf=yes
],[
    AC_MSG_RESULT(no)
])

have_tx_ring=no
dnl Check for older Linux TX_RING support
AC_MSG_CHECKING(for TX_RING socket sending support)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>     /* the L2 protocols */
#include <netinet/in.h>       /* htons */
#include <linux/if_packet.h>
]], [[
    int test;
    test = TP_STATUS_WRONG_FORMAT
]])],[
    AC_DEFINE([HAVE_TX_RING], [1],
            [Do we have Linux TX_RING socket support?])
    AC_MSG_RESULT(yes)
    have_tx_ring=yes
],[
    AC_MSG_RESULT(no)
])


AC_CHECK_HEADERS([net/bpf.h], [have_bpf=yes], [have_bpf=no])
if test $have_bpf = yes ; then
    AC_DEFINE([HAVE_BPF], [1],
            [Do we have BPF device support?])
fi

dnl ##################################################
dnl # Check for libdnet, but only if not Cygwin!
dnl #
dnl # if your version of libdnet doesn't compile, try
dnl # getting it from ...
dnl # http://libdnet.sourceforge.net/
dnl # ... or ...
dnl # https://github.com/jncornett/libdnet
dnl # ... or ...
dnl # on Linux try 'apt-get install libdumbnet-dev'
dnl ##################################################
founddnet=no
have_libdnet=no
libdnet_version=
enable_fragroute=no
trydnetdir=/usr/local

if test $have_cygwin = no ; then
    AC_MSG_CHECKING(for libdnet)
    AC_ARG_WITH(libdnet,
        AS_HELP_STRING([--with-libdnet=DIR],[Use libdnet in DIR]),
        [trydnetdir=$withval])

    if test $trydnetdir != no; then
        case "$build_os" in
        linux*)
            dnl # Debian/Ubuntu already have a package called libdnet
            dnl # so you the package you want to install libdumbnet-dev
            for testdir in $trydnetdir /usr/local /opt/local /usr ; do
                if test -x ${testdir}/bin/dumbnet-config -a $founddnet = no ; then
                    LDNETINC="$($testdir/bin/dumbnet-config --cflags)"
                    LDNETLIB="$($testdir/bin/dumbnet-config --libs)"
                    libdnet_version="$($testdir/bin/dumbnet-config --version) (libdumbnet)"
                    founddnet=$testdir
                fi
            done
            ;;
        esac

        if test $founddnet = no ; then
            for testdir in $trydnetdir /usr/local /opt/local $MACOSX_SDK_PATH/usr /usr ; do
                if test -x ${testdir}/bin/dnet-config -a $founddnet = no ; then
                    LDNETINC="$($testdir/bin/dnet-config --cflags)"
                    LDNETLIB="$($testdir/bin/dnet-config --libs)"
                    libdnet_version="$($testdir/bin/dnet-config --version)"
                    founddnet=$testdir
                fi
            done
        fi
    fi

    if test $founddnet = no ; then
        AC_MSG_RESULT(no)
        AC_MSG_WARN([libdnet not found, disabling fragroute feature])
    else
        AC_MSG_RESULT($founddnet)
        enable_fragroute=yes
        have_libdnet=yes
        AC_DEFINE([HAVE_LIBDNET], [1], [Do we have libdnet?])
        dnl Debian uses dumbnet.h instead of dnet.h due to naming conflict
        old_cflags=$CFLAGS
        CFLAGS=$LDNETINC
        AC_CHECK_HEADERS([dumbnet.h dnet.h])
        CFLAGS=$old_cflags
    fi

else
    AC_MSG_NOTICE([Skipping libdnet check under Cygwin for compatibility reasons])
fi


AC_SUBST(LDNETINC)
AC_SUBST(LDNETLIB)
AC_DEFINE_UNQUOTED([LIBDNET_VERSION], "$libdnet_version", [Version of libdnet])

AM_CONDITIONAL(COMPILE_FRAGROUTE, [test x$founddnet != xno])
if test x$founddnet != xno ; then
    AC_DEFINE(ENABLE_FRAGROUTE, [1], [Enable fragroute module])
fi


dnl Make sure we have a valid packet injection mechanisim
if test $have_bpf = no -a $have_pcap_inject = no -a $have_pcap_sendpacket = no \
    -a $have_libdnet = no -a $have_pf = no ; then
    AC_MSG_ERROR([Unable to find a supported method to send packets.  Please upgrade your libpcap or enable libdnet])
fi

dnl Older versions of libpcap are missing some DLT types
dnl If doesn't exist, we'll define them in src/common/fakepcap.h
AC_MSG_CHECKING(for DLT_LINUX_SLL in libpcap)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include "$LPCAPINC"]], [[ int foo;
                 foo = DLT_LINUX_SLL
               ]])],[ AC_DEFINE([HAVE_DLT_LINUX_SLL], [1],
                           [Does pcap.h include a header with DLT_LINUX_SLL?])
                 AC_MSG_RESULT(yes)
               ],[AC_MSG_RESULT(no)
               ])

AC_MSG_CHECKING(for DLT_C_HDLC in libpcap)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include "$LPCAPINC"]], [[ int foo;
                 foo = DLT_C_HDLC ]])],[ AC_DEFINE([HAVE_DLT_C_HDLC], [1],
                            [Does pcap.h include a header with DLT_C_HDLC?])
                  AC_MSG_RESULT(yes)
                ],[AC_MSG_RESULT(no)
               ])

PCAP_BPF_H_FILE="$LPCAPINCDIR/pcap-bpf.h"
AC_MSG_CHECKING(for $PCAP_BPF_H_FILE)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
                #include <sys/time.h>
                #include <stdint.h>
                #include "$PCAP_BPF_H_FILE"]], [[ int foo;
                 foo = BPF_MAJOR_VERSION; ]])],[ AC_DEFINE([INCLUDE_PCAP_BPF_HEADER], [1],
                           [What is the path (if any) to the pcap-bpf.h
                            header?])
                 AC_MSG_RESULT(yes)
                 AC_SUBST(PCAP_BPF_H_FILE)
               ],[AC_MSG_RESULT(no)
               ])

foundbpfheader=no
AC_MSG_CHECKING(for libpcap bpf header)
for testfile in $LPCAPINCDIR/pcap/bpf.h $LPCAPINCDIR/pcap-bpf.h ; do
    if test -f "${testfile}" -a $foundbpfheader = no ; then
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
            #include <sys/types.h>
            #include <sys/time.h>
            #include <stdint.h>
            #include "$testfile"]], [[
                int foo;
                foo = BPF_MAJOR_VERSION;
            ]])],[
                foundbpfheader=yes
                PCAP_BPF_H_FILE=$testfile
                AC_SUBST(PCAP_BPF_H_FILE)
                AC_DEFINE([INCLUDE_PCAP_BPF_HEADER], [1],
                        [What is the path (if any) to the libpcap bpf header file?])
            ],[])
    fi
done

if test $foundbpfheader != no; then
    AC_MSG_RESULT($testfile)
else
    AC_MSG_RESULT(no)
fi

dnl ## Test for netmap-libpcap
have_pcap_netmap=no
if test $foundpcap != no ; then
	AC_MSG_CHECKING(for netmap libpcap)
    PCAP_NETMAP_C_FILE="$LPCAPINCDIR/pcap-netmap.c"
    if test -f "${PCAP_NETMAP_C_FILE}" ; then
        AC_DEFINE([HAVE_LIBPCAP_NETMAP], [1],
                                   [Does this version of libpcap support netmap?])
        have_pcap_netmap=yes
        AC_MSG_RESULT(yes)
    else
        AC_MSG_RESULT(no)
    fi
fi

dnl restore LIBS & CFLAGS
LIBS="$OLDLIBS"
CFLAGS="$OLDCFLAGS"


dnl ##################################################
dnl # Check for pcapnav
dnl ##################################################
pcapnav_ver=no
pcncfg=no
AC_ARG_WITH(pcapnav-config,
    AS_HELP_STRING([--with-pcapnav-config=FILE],[Use given pcapnav-config]),
    [ AC_MSG_CHECKING(for pcapnav-config)
    if test -x $withval ; then
        pcncfg=$withval
        AC_MSG_RESULT($pcncfg)
    elif test x$withval = xno ; then
        AC_MSG_RESULT(no)
    else
        AC_MSG_RESULT([Can't find pcapnav-config: disabling offset jump feature])
    fi ],
    [ AC_PATH_PROG(pcncfg,[pcapnav-config], [no] ) ]
)

if test $pcncfg != no ; then
    LNAVLIB=$($pcncfg --libs)
    LNAV_CFLAGS=$($pcncfg --cflags)
    PCAPNAV_VERSION=$($pcncfg --version)
    AC_SUBST(LNAVLIB)
    AC_SUBST(LNAV_CFLAGS)
    AC_DEFINE_UNQUOTED(PCAPNAV_VERSION, "$PCAPNAV_VERSION",
                       [libpcapnav's version?])

dnl Check to see what version of libpcapnav
dnl this code has been reduced a lot, but probably still could be
dnl reduced quite a bit more if we chose too
    AC_MSG_CHECKING(for libpcapnav version)
    AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <string.h>
#define PCAPNAV_TEST "0.4"
/*
 * simple proggy to test the version of libpcapnav
 * returns zero if version >= 0.4
 * or one otherwise
 */

int
main (int argc, char *argv[])
{
    if (strncmp(PCAPNAV_VERSION, PCAPNAV_TEST, 3) >= 0)
        return 0;
    return 1;
}           ]])],[libpcapnav_ver=yes
            AC_MSG_RESULT(>= 0.4)],[libpcapnav_ver=no
            AC_MSG_RESULT(< 0.4)],[libpcapnav_ver=no
    ])


if test x$libpcapnav_ver = xno ; then
    AC_MSG_WARN([Libpcapnav versions < 0.4 are not supported.
    Please upgrade to version 0.4 or better.
    Disabling offset jump feature.])
else
    AC_DEFINE([HAVE_PCAPNAV], [1], [Do we have libpcapnav?])
fi

fi # checking pcapnav version


dnl (shamelessly ripped off from libpcap)
dnl Checks to see if unaligned memory accesses fail
dnl
dnl     FORCE_ALIGN (DEFINED)
dnl
AC_MSG_CHECKING(for requires strict byte alignment)
AC_CACHE_VAL(unaligned_cv_fail,
[
case "$host_os" in
    darwin*)
        # macOS M1 test fails due to lack of fork() so just hard code it
        unaligned_cv_fail=no
    ;;
    *)
        case "$host_cpu" in

        # XXX: should also check that they don't do weird things
        alpha*|arm*|hp*|mips*|sparc*|ia64)
                unaligned_cv_fail=yes
                ;;

        *)
        cat >conftest.c <<EOF
        #include <sys/types.h>
        #include <sys/wait.h>
        #include <stdio.h>
        #include <unistd.h>
        unsigned char a[[5]] = { 1, 2, 3, 4, 5 };
        int main() {
            unsigned int i;
            pid_t pid;
            int status;
            /* avoid "core dumped" message */
            pid = fork();
            if (pid <  0)
                return 2;
            if (pid > 0) {
                /* parent */
                pid = waitpid(pid, &status, 0);
                if (pid < 0)
                        return 3;
                return !WIFEXITED(status);
            }
            /* child */
            i = *(unsigned int *)&a[[1]];
            printf("%d\n", i);
            return 0;
        }
EOF
        ${CC-cc} -o conftest $CFLAGS $CPPFLAGS $LDFLAGS \
            conftest.c $LIBS >/dev/null 2>&1
        if test ! -x conftest ; then
            dnl failed to compile for some reason
            unaligned_cv_fail=yes
        else
            ./conftest >conftest.out
            if test ! -s conftest.out ; then
                unaligned_cv_fail=yes
            else
                unaligned_cv_fail=no
            fi
        fi
        rm -f conftest* core core.conftest
        ;;

        esac
    ;;
    esac
])
AC_MSG_RESULT($unaligned_cv_fail)
if test $unaligned_cv_fail = yes ; then
    AC_DEFINE([FORCE_ALIGN], [1], [Are we strictly aligned?])
fi

dnl ##################################################
dnl # Check for tcpdump.
dnl ##################################################

tcpdump_path=no
AC_ARG_WITH(tcpdump,
    AS_HELP_STRING([--with-tcpdump=FILE],[Path to tcpdump binary]),
    [ if test -x $withval ; then
          tcpdump_path=$withval
          AC_MSG_RESULT([Using tcpdump in $tcpdump_path])
      else
          AC_MSG_RESULT([Error: $withval does not exist or is not executable])
      fi ],
    [ AC_PATH_PROG(tcpdump_path, tcpdump, "no", [$PATH:/usr/sbin:/sbin:/usr/local/sbin]) ])


if test "$tcpdump_path" = "no"; then
    AC_MSG_WARN([Unable to find tcpdump.  Please specify --with-tcpdump.
                 Disabling --verbose])
else
    AC_DEFINE([HAVE_TCPDUMP], [1], [Do we have tcpdump?])
    AC_DEFINE_UNQUOTED(TCPDUMP_BINARY, "$tcpdump_path", [The tcpdump binary initially used])
fi


AM_CONDITIONAL([ENABLE_TCPDUMP], test "$tcpdump_path" != "no" -a x$have_pcap_dump_fopen = xyes)
if test x$tcpdump_path != xno -a x$have_pcap_dump_fopen = xyes ; then
    AC_DEFINE([ENABLE_VERBOSE], [1], [Do we have tcpdump and pcap_dump_fopen()?])
else
    AC_MSG_WARN([Your version of libpcap does not support --verbose])
fi

dnl No 'make test' when cross compile

AC_MSG_CHECKING(for 'make test' profile)
if test "$host" = "$build" ; then
    AC_MSG_RESULT([yes])
else    
    AC_MSG_WARN(Unable to do tests when cross-compiling)
    AC_MSG_RESULT([Unable to do tests when cross-compiling])
fi

dnl Allows user to choose which nic to use for testing purposes
AC_ARG_WITH(testnic,
    AS_HELP_STRING([--with-testnic=NIC],[Select which network card to use for testing]),
    [ nic1=$withval
      nic2=$withval
      AC_MSG_RESULT([Using --with-testnic=$withval])],
    [
case $host in
    *-*-linux*)
    nic1=eth0
    nic2=eth0
    ;;

    *-*-solaris*)
    nic1=hme0
    nic2=hme0
    ;;

    *-*-sunos*)
    nic1=hme0
    nic2=hme0
    ;;

    *-apple-darwin*)
    nic1=en0
    nic2=en0
    ;;

    *-*-openbsd*)
    nic1=xl0
    nic2=xl0
    ;;

    *-*-freebsd*)
    nic1=em0
    nic2=em0
    ;;

    *-*-cygwin)
    nic1=%0
    nic2=%0
    ;;

    *)
    AC_MSG_RESULT([$host is unknown!  Using first non-loopback interface])
    nic1=%0
    nic2=%0
    ;;
esac])

dnl There's a bug in OS X which causes pcap_findalldevs() to make the wifi NIC to disassociate
dnl so under OSX we disable the interface list feature
disable_pcap_findalldevs=no
osx_frameworks=no

AC_MSG_CHECKING([for OS version])
dnl these need to be dynamic based on OS
case $host in
    *-*-linux*)
    AC_DEFINE([HAVE_LINUX], [1], [Building Linux])
    AC_MSG_RESULT(Linux)
    ;;

    *-*-solaris*)
    AC_DEFINE([HAVE_SOLARIS], [1], [Building Solaris])
    AC_MSG_RESULT(Solaris)
    ;;

    *-*-sunos*)
    AC_DEFINE([HAVE_SUNOS], [1], [Building SunOS])
    AC_MSG_RESULT(SunOS)
    ;;

    *-apple-darwin*)
    if test x$libpcap_version_096 = xno ; then
        disable_pcap_findalldevs=yes
    fi
    osx_frameworks=yes
    AC_DEFINE([HAVE_DARWIN], [1], [Building Apple/Darwin])
    AC_MSG_RESULT(Apple macOS)
    ;;

    *-*-openbsd*)
    AC_DEFINE([HAVE_OPENBSD], [1], [Building Open BSD])
    AC_MSG_RESULT(OpenBSD)
    ;;

    *-*-freebsd*)
    AC_DEFINE([HAVE_FREEBSD], [1], [Building Free BSD])
    AC_MSG_RESULT(FreeBSD)
    ;;

    *-*-cygwin)
    AC_MSG_RESULT(Win32/Cygwin)
    AC_DEFINE([HAVE_CYGWIN], [1], [Building Cygwin])
    ;;

    *)
    AC_MSG_RESULT([$host is unknown!])
    ;;
esac
AM_CONDITIONAL([ENABLE_OSX_FRAMEWORKS], test "$osx_frameworks" = "yes")

AC_ARG_WITH(testnic2,
    AS_HELP_STRING([--with-testnic2=NIC2],[Select an optional 2nd network card to use for testing]),
    [ nic2=$withval ])

AC_MSG_NOTICE([Using $nic1 for 1st test network interface card])
AC_MSG_NOTICE([Using $nic2 for 2nd test network interface card])
AC_SUBST(nic1)
AC_SUBST(nic2)


AC_MSG_CHECKING([if it's ok to use pcap_findalldevs()])
if test x$disable_pcap_findalldevs = xno ; then
    AC_DEFINE([ENABLE_PCAP_FINDALLDEVS], [1], [Enable use of pcap_findalldevs()])
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi

LIBOPTS_CHECK(libopts)


AC_CONFIG_FILES([Makefile
           doxygen.cfg
           lib/Makefile
           docs/Makefile
           src/Makefile
           src/tcpedit/Makefile
           src/fragroute/Makefile
           src/common/Makefile
           src/defines.h
           test/Makefile
           test/config
           scripts/Makefile])

AC_OUTPUT

# Configuration results
AC_MSG_RESULT([
##########################################################################
             TCPREPLAY Suite Configuration Results (${TCPREPLAY_VERSION})
##########################################################################
libpcap:                    ${foundpcap} (${libpcap_version})
PF_RING libpcap             ${pf_ring_found}   ${pf_ring_lib}
libdnet:                    ${founddnet}   ${libdnet_version}
autogen:                    ${AUTOGEN} (${AUTOGEN_VERSION})
Use libopts tearoff:        ${enable_local_libopts}
64bit counter support:      ${use64bit_counters}
tcpdump binary path:        ${tcpdump_path}
fragroute support:          ${enable_fragroute}
tcpbridge support:          ${enable_tcpbridge}
tcpliveplay support:        ${enable_tcpliveplay}

Supported Packet Injection Methods (*):
Linux TX_RING:              ${have_tx_ring}
Linux PF_PACKET:            ${have_pf}
BSD BPF:                    ${have_bpf}
libdnet:                    ${have_libdnet}
pcap_inject:                ${have_pcap_inject}
pcap_sendpacket:            ${have_pcap_sendpacket} **
pcap_netmap                 ${have_pcap_netmap}
Linux/BSD netmap:           ${have_netmap}
Tuntap device support:      ${have_tuntap}

* In order of preference; see configure --help to override
** Required for tcpbridge
])


case $host in
    *-*-cygwin)
        AC_MSG_WARN([Windows/Cygwin support is still somewhat experimental.
Please report any bugs! https://github.com/appneta/tcpreplay/issues])
    ;;
esac

