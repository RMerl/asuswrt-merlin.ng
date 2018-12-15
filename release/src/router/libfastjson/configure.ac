AC_PREREQ(2.52)

# Process this file with autoconf to produce a configure script.
AC_INIT([libfastjson], [0.99.8], [rsyslog@lists.adiscon.com])
# AIXPORT START: Detect the underlying OS
unamestr=$(uname)
AM_CONDITIONAL([AIX], [test x$unamestr = xAIX])
# AIXPORT END

# AIXPORT : Set the required variables for AIX config script
if test "$unamestr" = "AIX"; then
	export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig:/usr/lib/pkgconfig"
	LIBS="-lbsd -lm"
	CPPFLAGS="-D_AIX -D_THREAD_SAFE -D_BSD=43"
	LDFLAGS="-qcpluscmt -brtl -bexpall "
	CC="xlc"
 AC_PREFIX_DEFAULT(/usr)
fi
# AIXPORT END


AC_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC_C99
AC_PROG_MAKE_SET
m4_ifdef([AC_USE_SYSTEM_EXTENSIONS], [AC_USE_SYSTEM_EXTENSIONS])

if test "$GCC" = "yes"
then
	m4_ifdef([AX_IS_RELEASE], [
		AX_IS_RELEASE([git-directory])
		m4_ifdef([AX_COMPILER_FLAGS], [
			AX_COMPILER_FLAGS()
		], [
			CFLAGS="$CFLAGS -W -Wall -Wformat-security -Wshadow -Wcast-align -Wpointer-arith -Wmissing-format-attribute -g"
			AC_MSG_WARN([missing AX_COMPILER_FLAGS macro, not using it])
		])
	], [
		CFLAGS="$CFLAGS -W -Wall -Wformat-security -Wshadow -Wcast-align -Wpointer-arith -Wmissing-format-attribute -g"
		AC_MSG_WARN([missing AX_IS_RELEASE macro, not using AX_COMPILER_FLAGS macro because of this])
	])
else
	AC_MSG_WARN([compiler is not GCC or close compatible, not using ax_compiler_flags because of this (CC=$CC)])
fi

AC_ARG_ENABLE(rdrand,
 AS_HELP_STRING([--enable-rdrand],
   [Enable RDRAND Hardware RNG Hash Seed generation on supported x86/x64 platforms.]),
[if test x$enableval = xyes; then
  enable_rdrand=yes
  AC_DEFINE(ENABLE_RDRAND, 1, [Enable RDRANR Hardware RNG Hash Seed])
fi])

if test "x$enable_rdrand" = "xyes"; then
  AC_MSG_RESULT([RDRAND Hardware RNG Hash Seed enabled on supported x86/x64 platforms])
else
  AC_MSG_RESULT([RDRAND Hardware RNG Hash Seed disabled. Use --enable-rdrand to enable])
fi

# enable silent build by default
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_ARG_ENABLE(valgrind,
        [AS_HELP_STRING([--enable-valgrind],[enable running the testbench under valgrind memcheck tool @<:@default=no@:>@])],
        [case "${enableval}" in
         yes) enable_valgrind="yes" ;;
          no) enable_valgrind="no" ;;
           *) AC_MSG_ERROR(bad value ${enableval} for --enable-valgrind) ;;
         esac],
        [enable_valgrind="no"]
)
if test "$enable_valgrind" = "yes"; then
        AC_CHECK_PROG(VALGRIND, [valgrind], [valgrind], [no])
	if test "x$VALGRIND" = "xno"; then
		AC_MSG_ERROR([--enable-valgrind given, but valgrind is not present on system])
	fi
	AC_SUBST(VALGRIND)
	AC_DEFINE(VALGRIND, 1, [valgrind enabled])
fi

# Checks for programs.

# Checks for libraries.

# Checks for header files.
AM_PROG_CC_C_O
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h limits.h strings.h syslog.h unistd.h [sys/cdefs.h] [sys/param.h] stdarg.h locale.h endian.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T


# check for availability of atomic operations
RS_ATOMIC_OPERATIONS
RS_ATOMIC_OPERATIONS_64BIT


# Checks for library functions.
AC_FUNC_VPRINTF
AC_FUNC_MEMCMP
AC_CHECK_FUNCS(strcasecmp strdup strerror snprintf vsnprintf vasprintf open vsyslog strncasecmp setlocale)

if test "$ac_cv_have_decl_isnan" = "yes" ; then
   AC_TRY_LINK([#include <math.h>], [float f = 0.0; return isnan(f)], [], [LIBS="$LIBS -lm"])
fi

#check if .section.gnu.warning accepts long strings (for __warn_references)
AC_LANG_PUSH([C])

AC_MSG_CHECKING([if .gnu.warning accepts long strings])
AC_LINK_IFELSE([AC_LANG_SOURCE([[
extern void json_object_get();
__asm__(".section .gnu.json_object_get,\n\t.ascii \"Please link against libfastjson instead of libjson\"\n\t.text");

int main(int c,char* v) {return 0;}
]])], [
    AC_DEFINE(HAS_GNU_WARNING_LONG, 1, [Define if .gnu.warning accepts long strings.])
    AC_MSG_RESULT(yes)
], [
   AC_MSG_RESULT(no)
])

AC_LANG_POP([C])

AM_PROG_LIBTOOL

# Check for the -Bsymbolic-functions linker flag
AC_ARG_ENABLE([Bsymbolic],
              [AS_HELP_STRING([--disable-Bsymbolic], [Avoid linking with -Bsymbolic-function])],
              [],
              [enable_Bsymbolic=check])

AS_IF([test "x$enable_Bsymbolic" = "xcheck"],
      [
        saved_LDFLAGS="${LDFLAGS}"
        AC_MSG_CHECKING([for -Bsymbolic-functions linker flag])
        LDFLAGS=-Wl,-Bsymbolic-functions
        AC_TRY_LINK([], [int main (void) { return 0; }],
                    [
                      AC_MSG_RESULT([yes])
                      enable_Bsymbolic=yes
                    ],
                    [
                      AC_MSG_RESULT([no])
                      enable_Bsymbolic=no
                    ])
        LDFLAGS="${saved_LDFLAGS}"
      ])

AS_IF([test "x$enable_Bsymbolic" = "xyes"], [JSON_BSYMBOLIC_LDFLAGS=-Wl[,]-Bsymbolic-functions])
AC_SUBST(JSON_BSYMBOLIC_LDFLAGS)

AC_CONFIG_FILES([
Makefile
libfastjson.pc
libfastjson-uninstalled.pc
tests/Makefile
])

AC_OUTPUT

