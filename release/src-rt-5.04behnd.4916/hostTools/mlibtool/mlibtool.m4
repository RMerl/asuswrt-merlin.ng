# autoconf macros for mlibtool

# Copyright (c) 2013 Gregor Richards
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

AC_DEFUN([ACX_MLT_INIT], [
    # First just init the normal GNU libtool
    LT_INIT($@)

    # Try to compile mlibtool
    if cc -O "$srcdir/mlibtool.c" -o mlibtool > /dev/null 2> /dev/null && \
       $ac_pwd/mlibtool --help > /dev/null 2> /dev/null
    then
        MLIBTOOL="$ac_pwd/mlibtool"

        # Find flags for mlibtool
        if test "x$enable_shared" = "xyes"; then
            MLIBTOOL="$MLIBTOOL --enable-shared"
        fi
        if test "x$enable_static" = "xyes"; then
            MLIBTOOL="$MLIBTOOL --enable-static"
        fi

        # And let mlibtool intercede
        LIBTOOL="$MLIBTOOL $LIBTOOL"
    fi
])
