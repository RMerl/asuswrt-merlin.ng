dnl Template file for GNU Autoconf
dnl Copyright (C) 1995-1997, 2001, 2007-2014, 2018-2024 Free Software
dnl Foundation, Inc.

dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 3 of the License, or
dnl (at your option) any later version.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.

dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.

dnl Additional permission under GNU GPL version 3 section 7

dnl If you modify this program, or any covered work, by linking or
dnl combining it with the OpenSSL project's OpenSSL library (or a
dnl modified version of that library), containing parts covered by the
dnl terms of the OpenSSL or SSLeay licenses, the Free Software Foundation
dnl grants you additional permission to convey the resulting work.
dnl Corresponding Source for a non-source form of such a combination
dnl shall include the source code for the parts of OpenSSL used as well
dnl as that of the covered work.

dnl
dnl Process this file with autoconf to produce a configure script.
dnl

dnl Error if AX_CODE_COVERAGE if missing (like on the 1.21 release).
m4_pattern_forbid([^_?AX_])
m4_pattern_allow([AX_CHECK_GNU_MAKE_HEADLINE])
m4_pattern_allow([_AX_CODE_COVERAGE_GCOV_PROG_WITH])

AC_INIT([wget],[m4_esyscmd(build-aux/git-version-gen .tarball-version)],[bug-wget@gnu.org])
AC_PREREQ([2.64])

dnl
dnl What version of Wget are we building?
dnl
AC_MSG_NOTICE([configuring for GNU Wget $PACKAGE_VERSION])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AC_CONFIG_SRCDIR([src/wget.h])

dnl
dnl Automake setup
dnl
AM_INIT_AUTOMAKE([subdir-objects dist-lzip])

AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC

dnl
dnl Get canonical host
dnl
AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED([OS_TYPE], "$host_os",
                   [Define to be the name of the operating system.])

dnl Non-verbose make
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl
dnl Process External Libraries
dnl

dnl Libpsl: Public Suffix List checking
AC_ARG_WITH([libpsl],
  [AS_HELP_STRING([--without-libpsl], [disable support for libpsl cookie checking.])])

dnl SSL: Configure SSL backend to use
AC_ARG_WITH([ssl],
  [AS_HELP_STRING([--with-ssl={gnutls,openssl,no}], [specify SSL backend. GNU TLS is the default.])])

dnl Zlib: Configure use of zlib for compression
AC_ARG_WITH([zlib],
  [AS_HELP_STRING([--without-zlib], [disable zlib.])])

dnl Metalink: Configure use of the Metalink library
AC_ARG_WITH([metalink],
  [AS_HELP_STRING([--with-metalink], [enable support for metalinks.])])

dnl C-Ares: Configure use of the c-ares library for DNS lookup
AC_ARG_WITH(cares, AS_HELP_STRING([--with-cares], [enable support for C-Ares DNS lookup.]), with_cares=$withval, with_cares=no)

dnl
dnl Process features
dnl

AC_ARG_ENABLE([fuzzing],
  [AS_HELP_STRING([--enable-fuzzing], [Turn on fuzzing build (for developers)])],
  [enable_fuzzing=yes;
   AC_SUBST([LIB_FUZZING_ENGINE])
   AC_DEFINE([FUZZING], 1, [Define to 1 if this is a fuzzing build])
  ], [enable_fuzzing=no; LIB_FUZZING_ENGINE=""])
AM_CONDITIONAL([FUZZING], [test "$enable_fuzzing" = "yes"])

dnl needed for some fuzzers
OLD_LIBS="$LIBS"
AC_SEARCH_LIBS([dlopen], [dl dld])
if [[ "${ac_cv_search_dlopen#-l}" != "${ac_cv_search_dlopen}" ]]; then
  FUZZ_LIBS="${ac_cv_search_dlopen}"
  AC_SUBST([FUZZ_LIBS])
fi
LIBS="$OLD_LIBS"


AC_ARG_ENABLE([fsanitize-ubsan],
  [AS_HELP_STRING([--enable-fsanitize-ubsan], [Turn on Undefined Behavior Sanitizer (for developers)])],
  [gl_cc_sanitize_ubsan=yes], [gl_cc_sanitize_ubsan=no])

AC_ARG_ENABLE([fsanitize-asan],
  [AS_HELP_STRING([--enable-fsanitize-asan], [Turn on Address Sanitizer (for developers) (mutually exclusive with Memory/Thread sanitizer or Valgrind tests)])],
  [gl_cc_sanitize_asan=yes], [gl_cc_sanitize_asan=no])

AC_ARG_ENABLE([fsanitize-msan],
  [AS_HELP_STRING([--enable-fsanitize-msan], [Turn on Memory Sanitizer (for developers) (mutually exclusive with Address/Thread sanitizer or Valgrind tests)])],
  [gl_cc_sanitize_msan=yes], [gl_cc_sanitize_msan=no])

if test "$gl_cc_sanitize_asan" = yes; then
  if test "$gl_cc_sanitize_msan" = yes; then
    AC_MSG_ERROR([Address Sanitizer and Memory Sanitizer are mutually exclusive])
  fi
fi

if test "$gl_cc_sanitize_ubsan" = yes; then
  gl_WARN_ADD([-fsanitize=undefined])
  gl_WARN_ADD([-fno-sanitize-recover=undefined])

  # additional clang options
  gl_WARN_ADD([-fsanitize=integer])
  gl_WARN_ADD([-fsanitize=nullability])
  gl_WARN_ADD([-fno-sanitize-recover=nullability])
fi

if test "$gl_cc_sanitize_asan" = yes; then
  gl_WARN_ADD([-fsanitize=address])
  gl_WARN_ADD([-fno-omit-frame-pointer])

  # additional clang option
  gl_WARN_ADD([-fsanitize-address-use-after-scope])
fi

if test "$gl_cc_sanitize_msan" = yes; then
  # clang options
  gl_WARN_ADD([-fsanitize=memory])
  gl_WARN_ADD([-fsanitize-memory-track-origins])

  # gcc options
  if test "$CCNAME" = "gcc"; then
    gl_WARN_ADD([-fsanitize=leak])
  fi

  gl_WARN_ADD([-fno-omit-frame-pointer])
  gl_WARN_ADD([-fPIE])
fi


dnl Opie: Support for opie s/key FTP logins
AC_ARG_ENABLE([opie],
  [AS_HELP_STRING([--disable-opie], [disable support for opie or s/key FTP login])],
  [ENABLE_OPIE=$enableval],
  [ENABLE_OPIE=yes])

AS_IF([test "x$ENABLE_OPIE" = xyes],
  [AC_DEFINE([ENABLE_OPIE], [1], [Define if you want Opie support for FTP compiled in.])],
  []
)


dnl Digest: Support for HTTP Digest Authentication
AC_ARG_ENABLE([digest],
  [AS_HELP_STRING([--disable-digest], [disable support for HTTP digest authorization])],
  [ENABLE_DIGEST=$enableval],
  [ENABLE_DIGEST=yes])

AS_IF([test "x$ENABLE_DIGEST" = xyes],
  [AC_DEFINE([ENABLE_DIGEST], [1], [Define if you want the HTTP Digest Authorization compiled in.])],
  []
)


dnl NTLM: Support for HTTP NTLM Authentication
AC_ARG_ENABLE([ntlm],
  [AS_HELP_STRING([--disable-ntlm], [disable support for NTLM authorization])],
  [ENABLE_NTLM=$enableval],
  [ENABLE_NTLM=auto]
)


dnl Debug: Support for printing debugging output
AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--disable-debug], [disable support for debugging output])],
  [ENABLE_DEBUG=$enableval],
  [ENABLE_DEBUG=yes])

AS_IF([test "x$ENABLE_DEBUG" = xyes],
  [AC_DEFINE([ENABLE_DEBUG], [1], [Define if you want the debug output support compiled in.])],
  []
)

dnl Valgrind-tests: Should test suite be run under valgrind?
AC_ARG_ENABLE(valgrind-tests,
  [AS_HELP_STRING([--enable-valgrind-tests], [enable using Valgrind for tests])],
  [ENABLE_VALGRIND=$enableval],
  [ENABLE_VALGRIND=no])

AS_IF([test "x$ENABLE_VALGRIND" != xno], [
  AC_CHECK_PROG(HAVE_VALGRIND, valgrind, yes, no)
  AS_IF([test "x$HAVE_VALGRIND" = xyes], [
    if test "$gl_cc_sanitize_asan" = yes; then
      AC_MSG_ERROR([Valgrind and Address Sanitizer are mutually exclusive])
    elif test "$gl_cc_sanitize_msan" = yes; then
      AC_MSG_ERROR([Valgrind and Memory Sanitizer are mutually exclusive])
    fi

    VALGRIND_TESTS="1"
    AC_SUBST(VALGRIND_TESTS)
    VALGRIND_INFO="Test suite will be run under Valgrind"
  ], [
    VALGRIND_INFO="Valgrind not found"
  ])
], [
  VALGRIND_INFO="Valgrind testing not enabled"
])

AC_ARG_ENABLE(assert,
  [AS_HELP_STRING([--enable-assert], [enable assertions in code base])],
  [ENABLE_ASSERTION=$enableval],
  [ENABLE_ASSERTION=no]
)

AS_IF([test "x$ENABLE_ASSERTION" != xyes], [
  CFLAGS="-DNDEBUG $CFLAGS"
  ],
  [])

dnl
dnl Find the compiler
dnl

dnl We want these before the checks, so the checks can modify their values.
test -z "$CFLAGS"  && CFLAGS= auto_cflags=1
test -z "$CC" && cc_specified=yes

gl_EARLY

dnl
dnl Find python3
dnl
AM_PATH_PYTHON([3.0],,[:])
AM_CONDITIONAL([HAVE_PYTHON3], [test "$PYTHON" != :])

dnl
dnl Gettext
dnl
AM_GNU_GETTEXT([external],[need-ngettext])
AM_GNU_GETTEXT_VERSION([0.19.3])
AM_GNU_GETTEXT_REQUIRE_VERSION([0.19.3])

AC_PROG_RANLIB

AC_PROG_LEX([noyywrap])

dnl Turn on optimization by default.  Specifically:
dnl
dnl if the user hasn't specified CFLAGS, then
dnl   if compiler is gcc, then
dnl     use -O2 and some warning flags
dnl   else
dnl     use os-specific flags or -O
dnl
dnl For these tests, the system considers clang to be gcc
if test -n "$auto_cflags"; then
  if test -n "$GCC"; then
    CFLAGS="$CFLAGS -O2 -Wall -Wextra"
  else
    case "$host_os" in
      *hpux*)  CFLAGS="$CFLAGS +O3"                      ;;
      *ultrix* | *osf*) CFLAGS="$CFLAGS -O -Olimit 2000" ;;
      *)       CFLAGS="$CFLAGS -O" ;;
    esac
  fi
fi

dnl
dnl Checks for basic compiler characteristics.
dnl
AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE

dnl Check for large file support.  This check needs to come fairly
dnl early because it could (in principle) affect whether functions and
dnl headers are available, whether they work, etc.
AC_SYS_LARGEFILE
AC_CHECK_SIZEOF(off_t)

dnl
dnl Checks for system header files that might be missing.
dnl
AC_HEADER_STDBOOL
AC_CHECK_HEADERS(unistd.h sys/time.h)
AC_CHECK_HEADERS(termios.h sys/ioctl.h sys/select.h)
AC_CHECK_HEADERS(stdint.h inttypes.h pwd.h wchar.h dlfcn.h)

AC_CHECK_DECLS(h_errno,,,[#include <netdb.h>])

dnl
dnl Check sizes of integer types.  These are used to find n-bit
dnl integral types on older systems that fail to provide intN_t and
dnl uintN_t typedefs.
dnl
AC_CHECK_SIZEOF([long])

dnl
dnl Checks for non-universal or system-specific types.
dnl
AC_TYPE_SIZE_T
AC_TYPE_PID_T
AC_CHECK_TYPES([uint32_t, uintptr_t, intptr_t, int64_t])
AC_CHECK_TYPES(sig_atomic_t, [], [], [
#include <stdio.h>
#include <sys/types.h>
#include <inttypes.h>
#include <signal.h>
])

# gnulib, AM_ICONV should be placed before gl_INIT
AM_ICONV
gl_INIT

AX_CODE_COVERAGE

dnl
dnl Checks for library functions.
dnl
AC_FUNC_MMAP
AC_FUNC_FSEEKO
AC_CHECK_FUNCS(strptime timegm vsnprintf vasprintf drand48 pathconf)
AC_CHECK_FUNCS(strtoll usleep ftello sigblock sigsetjmp memrchr wcwidth mbtowc)
AC_CHECK_FUNCS(sleep symlink utime strlcpy random fmemopen)

dnl We expect to have these functions on Unix-like systems configure
dnl runs on.  The defines are provided to get them in config.h.in so
dnl Wget can still be ported to non-Unix systems (such as Windows)
dnl that lack some of these functions.
AC_DEFINE([HAVE_STRDUP], 1, [Define to 1 if you have the 'strdup' function.])
AC_DEFINE([HAVE_ISATTY], 1, [Define to 1 if you have the 'isatty' function.])

dnl Deal with specific hosts
case $host_os in
  *mingw32* )
    LIBS+=' -lws2_32'
    OS_USED="mswindows"
    ;;
esac
AM_CONDITIONAL([OS_MSWINDOWS], [test x"$OS_USED" = x"mswindows"])

# enable all possible compiler warnings in WARN_FLAGS
#
# to enable by default: create a file '.manywarnings'
# enable explicitly : ./configure --enable-manywarnings
# disable explicitly: ./configure --disable-manywarnings
sanitizer_flags="$WARN_CFLAGS"
wget_MANYWARNINGS(WARN_CFLAGS, C)
WARN_CFLAGS="$WARN_CFLAGS $sanitizer_flags"

if test -n "$WARN_CFLAGS"; then
  if test "$CCNAME" = "gcc"; then
    # Set up list of unwanted warnings
    nw=
    nw="$nw -Wsystem-headers"       # System headers may trigger lot's of useless warnings
    nw="$nw -Wvla"                  # This project is C99
    nw="$nw -Wstack-protector"
    nw="$nw -Wmissing-field-initializer"
    nw="$nw -Wtraditional"
    nw="$nw -Wtraditional-conversion"
    nw="$nw -Wc++-compat"
    nw="$nw -Wcast-qual"
    nw="$nw -Wconversion"
    nw="$nw -Wsign-conversion"
    nw="$nw -Wunsuffixed-float-constants"
    nw="$nw -Wdeclaration-after-statement" # C89 only, messing up gcc < 5
    nw="$nw -Wcast-function-type" # gcc 8, very noisy
    nw="$nw -Wabi" # gcc 8, very noisy
    nw="$nw -Wunused-macros" # triggers in auto-generated lex css parser, #pragma doesn't work, conflicts with -Werror
	nw="$nw -Wchkp" # Deprecated option
    nw="$nw -Wswitch-enum" # Too noisy
    nw="$nw -Wswitch-default" # TODO: Enable someday. Too noisy for now
    nw="$nw -Wpedantic" # GCC 11. Too noisy, will never be supported
    nw="$nw -Wnested-externs" # Stylistic choice and we do use it in hsts code
    nw="$nw -Wc90-c99-compat" # gcc-12, we use C99
    nw="$nw -Wlong-long" # gcc-12, we use C99
    nw="$nw -Wc99-c11-compat" # gnulib is at C11
    nw="$nw -Wuseless-cast" # gcc-14 warns about it (pretty useless for portable code)
    if test "$cross_compiling" = yes; then
      nw="$nw -Wformat"
    fi

    # remove unwanted warn flags
    wget_WORD_REMOVE([WARN_CFLAGS], [$WARN_CFLAGS], [$nw])

    WARN_CFLAGS="$WARN_CFLAGS -Wno-undef -Wno-float-equal"

    # add more flags as you like
    if test $GCC_VERSION -ge 5; then
      WARN_CFLAGS="$WARN_CFLAGS -fdiagnostics-color=always"
    fi
    if test "$cross_compiling" = yes; then
      WARN_CFLAGS="$WARN_CFLAGS -Wno-format"
    fi
    if test $GCC_VERSION -ge 8; then
      WARN_CFLAGS="$WARN_CFLAGS -Wno-cast-function-type"
    fi
    # WARN_CFLAGS="$WARN_CFLAGS -Werror"

    # We use a slightly smaller set of warning options for lib/.
    # Remove the following and save the result in GNULIB_WARN_CFLAGS.
    # Removing is not enough if these switches are implicitly set by other
    #  flags like -Wall or -Wextra. We have to explicitly unset them
    #  with -Wno-....
    nw=
    nw="$nw -Wpedantic"
    nw="$nw -Wsign-compare"
    nw="$nw -Wunused-parameter"
    nw="$nw -Wswitch-default"
    nw="$nw -Wformat-nonliteral"
    nw="$nw -Wsuggest-attribute=pure"
    nw="$nw -Wunsafe-loop-optimizations"
    nw="$nw -Wundef"
    nw="$nw -Wswitch-enum"
    nw="$nw -Wbad-function-cast"
    nw="$nw -Wredundant-decls"
    nw="$nw -Werror"
    wget_WORD_REMOVE([GNULIB_WARN_CFLAGS], [$WARN_CFLAGS], [$nw])
    wget_WORD_REMOVE([CFLAGS], [$CFLAGS], [-Werror])

    # disable options implicitly set by other options
    GNULIB_WARN_CFLAGS="-Wno-error $GNULIB_WARN_CFLAGS"
    GNULIB_WARN_CFLAGS="$GNULIB_WARN_CFLAGS -Wno-sign-compare -Wno-unused-parameter -Wno-alloca"
    GNULIB_WARN_CFLAGS="$GNULIB_WARN_CFLAGS -Wno-float-conversion -Wno-cast-function-type"
    if test "$cross_compiling" = yes; then
      GNULIB_WARN_CFLAGS="$GNULIB_WARN_CFLAGS -Wno-incompatible-pointer-types"
    fi

  elif test "$CCNAME" = "clang"; then
    # setup flags for this project
    WARN_CFLAGS="$WARN_CFLAGS -Wno-system-headers -Wno-vla -Wno-cast-qual -Wno-padded"
    WARN_CFLAGS="$WARN_CFLAGS -Wno-reserved-id-macro -Wno-sign-conversion  -Wno-disabled-macro-expansion"
    WARN_CFLAGS="$WARN_CFLAGS -Wno-documentation -Wno-documentation-unknown-command"
    WARN_CFLAGS="$WARN_CFLAGS -Wno-covered-switch-default -Wno-unused-macros"
    WARN_CFLAGS="$WARN_CFLAGS -Wno-missing-field-initializers"

    # remove all flags from WARN_FLAGS that are already in CFLAGS
    # wget_WORD_REMOVE([WARN_CFLAGS], [$WARN_CFLAGS], [$CFLAGS])

    wget_WORD_REMOVE([CFLAGS], [$CFLAGS], [-Werror])

    # disable verbose options
    GNULIB_WARN_CFLAGS="-Wno-error $GNULIB_WARN_CFLAGS"
    GNULIB_WARN_CFLAGS="$WARN_CFLAGS -Wno-sign-compare -Wno-unused-parameter -Wno-undef -Wno-format-nonliteral"
    GNULIB_WARN_CFLAGS="$GNULIB_WARN_CFLAGS -Wno-conversion -Wno-disabled-macro-expansion -Wno-c++98-compat"
    GNULIB_WARN_CFLAGS="$GNULIB_WARN_CFLAGS -Wno-gnu-statement-expression -Wno-shorten-64-to-32 -Wno-switch-enum"
    GNULIB_WARN_CFLAGS="$GNULIB_WARN_CFLAGS -Wno-unused-macros -Wno-missing-field-initializers"
    GNULIB_WARN_CFLAGS="$GNULIB_WARN_CFLAGS -Wno-gnu-zero-variadic-macro-arguments -Wno-conditional-uninitialized"
    GNULIB_WARN_CFLAGS="$GNULIB_WARN_CFLAGS -Wno-comma -Wno-assign-enum -Wno-unreachable-code -Wno-error"
    GNULIB_WARN_CFLAGS="$GNULIB_WARN_CFLAGS -Wno-missing-field-initializers"

    # remove all flags from GNULIB_WARN_FLAGS that are already in CFLAGS
    # wget_WORD_REMOVE([GNULIB_WARN_CFLAGS], [$GNULIB_WARN_CFLAGS], [$CFLAGS])
  fi

  AC_SUBST([WARN_CFLAGS])
  AC_SUBST([GNULIB_WARN_CFLAGS])
fi


dnl
dnl Checks for libraries.
dnl

PKG_PROG_PKG_CONFIG

AS_IF([test "x$with_libpsl" != xno], [
  PKG_CHECK_MODULES([LIBPSL], libpsl, [
    with_libpsl=yes
    # correct $LIBPSL_LIBS (in libpsl <= 0.6.0)
    AS_IF([test "x$LIBPSL_LIBS" = "x-llibpsl "], [LIBPSL_LIBS="-lpsl"])
    LIBS="$LIBPSL_LIBS $LIBS"
    CFLAGS="$LIBPSL_CFLAGS $CFLAGS"
    AC_DEFINE([HAVE_LIBPSL], [1], [PSL support enabled])
  ], [
    AC_SEARCH_LIBS(psl_builtin, psl,
      [with_libpsl=yes; AC_DEFINE([HAVE_LIBPSL], [1], [PSL support enabled])],
      [with_libpsl=no;  AC_MSG_WARN(*** libpsl was not found. Fallback to builtin cookie checking.)])
  ])
])

AS_IF([test x"$with_libpsl" = xyes], [
  AC_CHECK_FUNCS(psl_latest)
])

AS_IF([test x"$with_zlib" != xno], [
  with_zlib=yes
  PKG_CHECK_MODULES([ZLIB], zlib, [
    LIBS="$ZLIB_LIBS $LIBS"
    CFLAGS="$ZLIB_CFLAGS $CFLAGS"
    AC_DEFINE([HAVE_LIBZ], [1], [Define if using zlib.])
  ], [
    AC_CHECK_LIB(z, compress)
  ])
])

AS_IF([test x"$with_ssl" = xopenssl], [
  if [test x"$with_libssl_prefix" = x]; then
    PKG_CHECK_MODULES([OPENSSL], [openssl], [
      AC_MSG_NOTICE([compiling in support for SSL via OpenSSL])
      ssl_library="openssl"
      LIBS="$OPENSSL_LIBS $LIBS"
      CFLAGS="$OPENSSL_CFLAGS -DHAVE_LIBSSL $CFLAGS"
      LIBSSL=" " # ntlm check below wants this
      AC_CHECK_FUNCS([RAND_egd])
      AC_DEFINE([HAVE_LIBSSL], [1], [Define if using openssl.])
      ssl_found=yes
    ])
  fi
  if [test x"$ssl_found" != xyes]; then
    dnl As of this writing (OpenSSL 0.9.6), the libcrypto shared library
    dnl doesn't record its dependency on libdl, so we need to make sure
    dnl -ldl ends up in LIBS on systems that have it.  Most OSes use
    dnl dlopen(), but HP-UX uses shl_load().
    AC_CHECK_LIB(dl, dlopen, [], [
      AC_CHECK_LIB(dl, shl_load)
    ])

    ssl_found=no
    case $host_os in
      *mingw32* )
        dnl prefer link to openssl dlls if possible. if not then fallback on static libs. if not then error

        AC_CHECK_LIB(eay32, EVP_MD_CTX_init)
        if test x"$ac_cv_lib_eay32_EVP_MD_CTX_init" != xno
        then
          AC_CHECK_LIB(ssl32, SSL_connect, [
            ssl_found=yes
            AC_MSG_NOTICE([Enabling support for SSL via OpenSSL (shared)])
            ssl_library="openssl"
            LIBS="${LIBS} -lssl32"
            AC_DEFINE([HAVE_LIBSSL32], [1], [Define to 1 if you have the 'ssl32' library (-lssl32).])
          ],
          AC_MSG_ERROR([openssl not found: shared lib eay32 found but ssl32 not found]))

        else
          LIBS+=' -lgdi32'
          dnl fallback and test static libs
        fi
        dnl add zdll lib as dep for above tests?
      ;;
    esac

    AS_IF([test x$ssl_found != xyes], [
      dnl Now actually check for -lssl if it wasn't already found
      AC_LIB_HAVE_LINKFLAGS([ssl], [crypto], [
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/des.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
      ], [SSL_library_init ()])
      if test x"$LIBSSL" != x
      then
        ssl_found=yes
        AC_MSG_NOTICE([compiling in support for SSL via OpenSSL])
        ssl_library="openssl"
        LIBS="$LIBSSL $LIBS"
        AC_CHECK_FUNCS([RAND_egd])
      elif test x"$with_ssl" != x
      then
        AC_MSG_ERROR([--with-ssl=openssl was given, but SSL is not available.])
      fi
    ])
  fi
], [
  # --with-ssl is not openssl: check if it's no
  AS_IF([test x"$with_ssl" != xno], [
    dnl default is -lgnutls
    with_ssl=gnutls

    dnl Now actually check for -lgnutls
    if [test x"$with_libgnutls_prefix" = x]; then
      PKG_CHECK_MODULES([GNUTLS], [gnutls], [
        AC_MSG_NOTICE([compiling in support for SSL via GnuTLS])
        ssl_library="gnutls"
        LIBS="$GNUTLS_LIBS $LIBS"
        CFLAGS="$GNUTLS_CFLAGS -DHAVE_LIBGNUTLS $CFLAGS"
        AC_DEFINE([HAVE_LIBGNUTLS], [1], [Define if using gnutls.])
        ssl_found=yes
      ])
    fi
    if [test x"$ssl_found" != xyes]; then
      AC_LIB_HAVE_LINKFLAGS([gnutls], [], [
#include <gnutls/gnutls.h>
      ], [gnutls_global_init()])
      if test x"$LIBGNUTLS" != x
      then
        ssl_found=yes
        AC_MSG_NOTICE([compiling in support for SSL via GnuTLS])
        ssl_library="gnutls"
        LIBS="$LIBGNUTLS $LIBS"
      else
        AC_MSG_ERROR([GnuTLS has not been found. Use --with-ssl=openssl if you explicitly want OpenSSL.])
      fi
    fi

    AC_CHECK_FUNCS(gnutls_priority_set_direct)
  ]) # endif: --with-ssl != no?
]) # endif: --with-ssl == openssl?

dnl Enable NTLM if requested and if SSL is available.
if test x"$ENABLE_NTLM" != xno
then
  if test x"$LIBSSL" != x || test "$ac_cv_lib_ssl32_SSL_connect" = yes
  then
    ENABLE_NTLM=yes
    AC_DEFINE([ENABLE_NTLM], 1, [Define if you want the NTLM authorization support compiled in.])
  else
    PKG_CHECK_MODULES([NETTLE], nettle, [
      HAVE_NETTLE=yes
      LIBS="$NETTLE_LIBS $LIBS"
      CFLAGS="$NETTLE_CFLAGS $CFLAGS"
    ], [
      AC_CHECK_LIB(nettle, nettle_md4_init, [HAVE_NETTLE=yes], [HAVE_NETTLE=no; AC_MSG_WARN(*** libnettle was not found. You will not be able to use NTLM)])
      if test x"$HAVE_NETTLE" != xyes; then
        if  test x"$ENABLE_NTLM" = xyes; then
          AC_MSG_ERROR([NTLM authorization requested and SSL not enabled; aborting])
        fi
      else
        AC_SUBST(NETTLE_LIBS, "-lnettle")
        LIBS="$NETTLE_LIBS $LIBS"
      fi
    ])

    if test x"$HAVE_NETTLE" = xyes; then
      AC_DEFINE([HAVE_NETTLE], [1], [Use libnettle])
      ENABLE_NTLM=yes
      AC_DEFINE([ENABLE_NTLM], 1, [Define if you want the NTLM authorization support compiled in.])
    fi
  fi
fi

dnl **********************************************************************
dnl Checks for IPv6
dnl **********************************************************************

dnl
dnl We test for IPv6 by checking, in turn, for availability of
dnl presence of the INET6 address/protocol family and the existence of
dnl struct sockaddr_in6.  If any of them is missing, IPv6 is disabled,
dnl and the code reverts to old-style gethostbyname.
dnl
dnl If --enable-ipv6 is explicitly specified on the configure command
dnl line, we check for IPv6 and abort if not found.  If --disable-ipv6
dnl is specified, we disable IPv6 and don't check for it.  The default
dnl is to autodetect IPv6 and use it where available.
dnl

AC_ARG_ENABLE(ipv6,
  AS_HELP_STRING([--disable-ipv6],[disable IPv6 support]),
  [case "${enable_ipv6}" in
    no)
      AC_MSG_NOTICE([disabling IPv6 at user request])
      dnl Disable IPv6 checking
      ipv6=no
      ;;
    yes)
      dnl IPv6 explicitly enabled: force its use (abort if unavailable).
      ipv6=yes
      force_ipv6=yes
      ;;
    auto)
      dnl Auto-detect IPv6, i.e. check for IPv6, but don't force it.
      ipv6=yes
      ;;
    *)
      AC_MSG_ERROR([Invalid --enable-ipv6 argument '$enable_ipv6'])
      ;;
    esac
  ], [
    dnl If nothing is specified, assume auto-detection.
    ipv6=yes
  ]
)

if test "X$ipv6" = "Xyes"; then
  PROTO_INET6([], [
    AC_MSG_NOTICE([Disabling IPv6 support: your system does not support the PF_INET6 protocol family])
    ipv6=no
  ])
fi

if test "X$ipv6" = "Xyes"; then
  TYPE_STRUCT_SOCKADDR_IN6([],[
    AC_MSG_NOTICE([Disabling IPv6 support: your system does not support 'struct sockaddr_in6'])
    ipv6=no
  ])
  if test "X$ipv6" = "Xyes"; then
    WGET_STRUCT_SOCKADDR_STORAGE
    MEMBER_SIN6_SCOPE_ID
  fi
fi

if test "X$ipv6" = "Xyes"; then
  AC_DEFINE([ENABLE_IPV6], 1, [Define if IPv6 support is enabled.])
  AC_MSG_NOTICE([Enabling support for IPv6.])
elif test "x$force_ipv6" = "xyes"; then
  AC_MSG_ERROR([IPv6 support requested but not found; aborting])
fi

dnl
dnl Find makeinfo.  We used to provide support for Emacs processing
dnl Texinfo using 'emacs -batch -eval ...' where makeinfo is
dnl unavailable, but that broke with the addition of makeinfo-specific
dnl command-line options, such as '-I'.  Now we depend on makeinfo to
dnl build the Info documentation.
dnl

AC_CHECK_PROGS(MAKEINFO, [makeinfo], [true])

dnl
dnl Find perl and pod2man
dnl

AC_PATH_PROGS(PERL, [perl5 perl], no)
AC_PATH_PROG(POD2MAN, pod2man, no)

if test "x${POD2MAN}" = xno; then
  COMMENT_IF_NO_POD2MAN="# "
else
  COMMENT_IF_NO_POD2MAN=
fi
AC_SUBST(COMMENT_IF_NO_POD2MAN)


dnl
dnl Check for IDN/IRIs
dnl

AC_ARG_ENABLE(iri,
  AS_HELP_STRING([--disable-iri],[disable IDN/IRIs support]),
  [case "${enable_iri}" in
    no)
      dnl Disable IRIs checking
      AC_MSG_NOTICE([disabling IRIs at user request])
      iri=no
      ;;
    yes)
      dnl IRIs explicitly enabled
      iri=yes
      force_iri=yes
      ;;
    auto)
      dnl Auto-detect IRI
      iri=yes
      ;;
    *)
      AC_MSG_ERROR([Invalid --enable-iri argument '$enable_iri'])
      ;;
    esac
  ], [
    dnl If nothing is specified, assume auto-detection
    iri=yes
  ]
)

AS_IF([test "X$iri" != "Xno"],[
  if test "X$am_cv_func_iconv" != "Xyes"; then
    iri=no
    if test "X$force_iri" = "Xyes"; then
      AC_MSG_ERROR([Libiconv is required for IRIs support])
    else
      AC_MSG_NOTICE([disabling IRIs because libiconv wasn't found])
    fi
  fi
])

if test "X$iri" != "Xno"; then
  AS_IF([test "x$with_libidn2" != xno], [
    PKG_CHECK_MODULES([LIBIDN2], [libidn2 >= 0.14.0], [
      with_libidn2=yes
      LIBS="$LIBIDN2_LIBS $LIBS"
      CFLAGS="$LIBIDN2_CFLAGS $CFLAGS"
      AC_DEFINE([ENABLE_IRI], 1, [Define if IRI support is enabled.])
    ], [
      AC_SEARCH_LIBS(idn2_lookup_u8, idn2,
        [with_libidn2=yes; AC_DEFINE([ENABLE_IRI], 1, [Define if IRI support is enabled.])],
        [with_libidn2=no; iri=no; AC_MSG_WARN(*** LIBIDN2 was not found. You will not be able to use IDN2008 support)])
    ])
  ])
fi

dnl
dnl Check for UUID
dnl

AC_ARG_WITH(libuuid, AS_HELP_STRING([--without-libuuid],[Generate UUIDs for WARC files via libuuid]))

AS_IF([test "x$with_libuuid" = xyes], [
  # libuuid was explicitly requested
  PKG_CHECK_MODULES([UUID], uuid, [
    LIBS="$UUID_LIBS $LIBS"
    CFLAGS="$UUID_CFLAGS $CFLAGS"
    uuid_mode=1
  ], [
    AC_SEARCH_LIBS(uuid_generate, uuid,
      [uuid_mode=1],
      [AC_MSG_ERROR(*** libuuid was explicitly requested but wasn't found.)])
  ])
], [test "x$with_libuuid" = xno], [
  # libuuid was explicitly *not* requested
  AC_CHECK_HEADER(uuid.h,
    AC_CHECK_FUNC(uuid_create, [uuid_mode=2]))
], [
  # default:
  AC_CHECK_HEADER(uuid.h, [
    AC_CHECK_FUNC(uuid_create, [uuid_mode=2])
  ], [
    AC_CHECK_HEADER(uuid/uuid.h,
      AC_SEARCH_LIBS(uuid_generate, uuid, [uuid_mode=1]))
  ])
])

AS_IF([test "x$uuid_mode" = x1], [
  AC_DEFINE([HAVE_LIBUUID], [1], [Define if using libuuid.])
], [test "x$uuid_mode" = x2], [
  AC_DEFINE([HAVE_UUID_CREATE], 1, [Define if uuid_create is available.])
])

dnl
dnl Check for PCRE2 / PCRE
dnl

AC_ARG_ENABLE(pcre2, AS_HELP_STRING([--disable-pcre2],[Disable PCRE2 style regular expressions]))
AC_ARG_ENABLE(pcre, AS_HELP_STRING([--disable-pcre],[Disable PCRE style regular expressions]))

AS_IF([test "X$enable_pcre2" != "Xno"],[
  enable_pcre2=no
  PKG_CHECK_MODULES([PCRE2], libpcre2-8, [
    CFLAGS="$PCRE2_CFLAGS $CFLAGS"
    LIBS="$PCRE2_LIBS $LIBS"
    AC_DEFINE([HAVE_LIBPCRE2], [1], [Define if libpcre2 is available.])
    enable_pcre2=yes
  ], [
    AC_CHECK_HEADER(pcre2.h, [
      AC_CHECK_LIB(pcre2-8, pcre2_compile_8, [
        LIBS="${LIBS} -lpcre2-8"
        AC_DEFINE([HAVE_LIBPCRE2], 1, [Define if libpcre2 is available.])
        enable_pcre2=yes
      ])
    ])
  ])
])

AS_IF([test "X$enable_pcre" != "Xno" && test "X$enable_pcre2" != "Xyes"],[
  PKG_CHECK_MODULES([PCRE], libpcre, [
    CFLAGS="$PCRE_CFLAGS $CFLAGS"
    AC_CHECK_HEADER(pcre.h, [
      LIBS="$PCRE_LIBS $LIBS"
      AC_DEFINE([HAVE_LIBPCRE], [1], [Define if libpcre is available.])
      enable_pcre=yes
    ])
  ], [
    AC_CHECK_HEADER(pcre.h, [
      AC_CHECK_LIB(pcre, pcre_compile, [
        LIBS="${LIBS} -lpcre"
        AC_DEFINE([HAVE_LIBPCRE], 1, [Define if libpcre is available.])
      ])
    ])
  ])
])

AS_IF([test "X$enable_pcre2" = Xyes], [PCRE_INFO="yes, via libpcre2"], [test "X$enable_pcre" = Xyes], [PCRE_INFO="yes, via libpcre"], [PCRE_INFO=no])

dnl
dnl Check for libcares (resolver library)
dnl

AS_IF([test "X$with_cares" = "Xyes"],[
  PKG_CHECK_MODULES([CARES], libcares, [
    CFLAGS="$CARES_CFLAGS $CFLAGS"
    AC_CHECK_HEADER(ares.h, [
      LIBS="$CARES_LIBS $LIBS"
      AC_DEFINE([HAVE_LIBCARES], [1], [Define if libcares is available.])
      RESOLVER_INFO="libcares, --bind-dns-address and --dns-servers available"
    ])
  ], [
    AC_CHECK_HEADER(ares.h, [
      AC_CHECK_LIB(cares, ares_set_local_ip4, [
        LIBS="-lcares ${LIBS}"
        AC_DEFINE([HAVE_LIBCARES], 1, [Define if libcares is available.])
        RESOLVER_INFO="libcares, --bind-dns-address and --dns-servers available"
      ])
    ])
  ])
], [
  RESOLVER_INFO="libc, --bind-dns-address and --dns-servers not available"
])

dnl
dnl Check for libmetalink
dnl
AS_IF([test x"$with_metalink" != xno], [
  PKG_CHECK_MODULES([METALINK], libmetalink, [
      LIBS="$METALINK_LIBS $LIBS"
      CFLAGS="$METALINK_CFLAGS $CFLAGS"
      AC_DEFINE([HAVE_METALINK], [1], [Define if using metalink.])
      with_metalink=yes
    ], [
      with_metalink=no
    ])

  have_gpg=no
  AS_IF([test x"$with_metalink" = xyes], [
    dnl
    dnl Check for GPGME
    dnl
    PKG_CHECK_MODULES([GPGME], gpgme, [
      LIBS="$LIBS $GPGME_LIBS"
      CFLAGS="$GPGME_CFLAGS $CFLAGS"
      AC_DEFINE([HAVE_GPGME], [1], [Define if GPGME is available.])
      have_gpg=yes
    ], [
      m4_ifdef([AM_PATH_GPGME], [
        AM_PATH_GPGME([], [
          # Put libgpgme to the end of the library list since it introduces a -L linker flags.
          # That -L might break the build if there are two different version of
          # a library (e.g. GnuTLS) in /usr/local and in the system directory.
          LIBS="$LIBS $GPGME_LIBS"
          CFLAGS="$GPGME_CFLAGS $CFLAGS"
          AC_DEFINE([HAVE_GPGME], [1], [Define if GPGME is available.])
          have_gpg=yes
        ])
      ])
    ])
  ])
])

dnl
dnl libproxy support
dnl
with_libproxy=no
AC_ARG_ENABLE(libproxy,
  [  --enable-libproxy       libproxy support for system wide proxy configuration])
AS_IF([test "${enable_libproxy}" = "yes"], [
  with_libproxy=yes
  PKG_CHECK_MODULES([LIBPROXY], [libproxy-1.0], [
    LIBS="$LIBPROXY_LIBS $LIBS"
    CFLAGS="$LIBPROXY_CFLAGS $CFLAGS"
    AC_DEFINE([HAVE_LIBPROXY], [1], [Define if using libproxy.])
  ])
])

dnl
dnl Extended Attribute support
dnl

AC_ARG_ENABLE([xattr],
  [AS_HELP_STRING([--disable-xattr], [disable support for POSIX Extended Attributes])],
  [ENABLE_XATTR=$enableval],
  [ENABLE_XATTR=yes])

case "$host_os" in
  *linux* | *darwin*) xattr_syscalls="fsetxattr" ;;
  freebsd*)           xattr_syscalls="extattr_set_fd" ;;
  *)  AC_MSG_NOTICE([Disabling Extended Attribute support: your system is not known to support extended attributes.])
      ENABLE_XATTR=no
esac

if test "X${ENABLE_XATTR}" = "Xyes"; then
  AC_CHECK_FUNCS([$xattr_syscalls], [], [
    AC_MSG_NOTICE([Disabling Extended Attribute support: your system does not support $xattr_syscalls])
    ENABLE_XATTR=no
  ])
fi

test "X${ENABLE_XATTR}" = "Xyes" && AC_DEFINE([ENABLE_XATTR], 1,
    [Define if you want file meta-data storing into POSIX Extended Attributes compiled in.])

dnl Needed by src/Makefile.am
AM_CONDITIONAL([WITH_IRI], [test "X$iri" != "Xno"])
AM_CONDITIONAL([WITH_SSL], [test "X$with_ssl" != "Xno"])
AM_CONDITIONAL([WITH_METALINK], [test "X$with_metalink" != "Xno"])
AM_CONDITIONAL([WITH_XATTR], [test "X$ENABLE_XATTR" != "Xno"])
AM_CONDITIONAL([WITH_NTLM], [test "X$ENABLE_NTLM" = "Xyes"])
AM_CONDITIONAL([WITH_OPIE], [test x"$ENABLE_OPIE" = x"yes"])
AM_CONDITIONAL([WITH_OPENSSL], [test x"$ssl_library" = x"openssl"])
AM_CONDITIONAL([WITH_GNUTLS], [test x"$ssl_library" = x"gnutls"])

dnl
dnl Create output
dnl
AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile util/Makefile
                 po/Makefile.in gnulib_po/Makefile.in tests/Makefile
                 fuzz/Makefile lib/Makefile testenv/Makefile
                 tests/certs/interca.conf tests/certs/rootca.conf])
AC_CONFIG_HEADERS([src/config.h])
AC_OUTPUT

AC_MSG_NOTICE([Summary of build options:

  Version:           $PACKAGE_VERSION
  Host OS:           $host_os
  Install prefix:    $prefix
  Compiler:          $CC
  CFlags:            $CFLAGS $CPPFLAGS $WARN_CFLAGS
  LDFlags:           $LDFLAGS
  Libs:              $LIBS
  SSL:               $with_ssl
  Zlib:              $with_zlib
  PSL:               $with_libpsl
  PCRE:              $PCRE_INFO
  Digest:            $ENABLE_DIGEST
  NTLM:              $ENABLE_NTLM
  OPIE:              $ENABLE_OPIE
  POSIX xattr:       $ENABLE_XATTR
  Debugging:         $ENABLE_DEBUG
  Assertions:        $ENABLE_ASSERTION
  Valgrind:          $VALGRIND_INFO
  Metalink:          $with_metalink
  Resolver:          $RESOLVER_INFO
  GPGME:             $have_gpg
  IRI:               $iri
  Fuzzing build:     $enable_fuzzing, $LIB_FUZZING_ENGINE
  libproxy:          $with_libproxy
])
