# compiler.m4 - autoconf macros for compiler settings
#
# Copyright © 2005 Scott James Remnant <scott@netsplit.com>.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
# ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


# COMPILER_WARNINGS
# ----------------------
# Add configure option to enable additional compiler warnings and treat
# them as errors.
AC_DEFUN([COMPILER_WARNINGS],
[AC_ARG_ENABLE(compiler-warnings,
	AS_HELP_STRING([--enable-compiler-warnings],
	               [Enable additional compiler warnings]),
[if test "x$enable_compiler_warnings" = "xyes"; then
	if test "x$GCC" = "xyes"; then
                CFLAGS="-Wall -Werror $CFLAGS"
        fi
	if test "x$GXX" = "xyes"; then
		CXXFLAGS="-Wall -Werror $CXXFLAGS"
	fi
fi])dnl
])# COMPILER_WARNINGS

# COMPILER_OPTIMISATIONS
# ---------------------------
# Add configure option to disable optimisations.
AC_DEFUN([COMPILER_OPTIMISATIONS],
[AC_ARG_ENABLE(compiler-optimisations,
	AS_HELP_STRING([--disable-compiler-optimisations],
		       [Disable compiler optimisations]),
[if test "x$enable_compiler_optimisations" = "xno"; then
	[CFLAGS=`echo "$CFLAGS" | sed -e "s/ -O[1-9]*\b/ -O0/g"`]
fi])dnl
])# COMPILER_OPTIMISATIONS

# COMPILER_COVERAGE
# ----------------------
# Add configure option to enable coverage data.
AC_DEFUN([COMPILER_COVERAGE],
[AC_ARG_ENABLE(compiler-coverage,
	AS_HELP_STRING([--enable-compiler-coverage],
		       [Enable generation of coverage data]),
[if test "x$enable_compiler_coverage" = "xyes"; then
	if test "x$GCC" = "xyes"; then
		CFLAGS="$CFLAGS -fprofile-arcs -ftest-coverage"
	fi
fi],[enable_compiler_coverage=no])dnl
])# COMPILER_COVERAGE
