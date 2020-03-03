#!/bin/bash

#
# (1) If your toolchain is not installed at /opt, run this script ** once **
#     to fix the installation.
# (2) Example: fix_tool_chain.sh /my_tool_chain/toolchains/uclibc-crosstools-gcc-4.2.3-3
#

default_toolchain_top=/opt/toolchains/uclibc-crosstools-gcc-4.2.3-3

if [ $# -ge 1 -a "$1" != "" -a "$1" != "$default_toolchain_top" ]; then
    cd $1/usr/lib
    mv libc.so libc.so.org
    echo "GROUP ( $1/lib/libc.so.0 $1/usr/lib/uclibc_nonshared.a  )" > libc.so
    ln -s ../../lib/ld-uClibc.so.0 ld-uClibc.so.0

    cd $1/usr/lib/gcc/mips-linux-uclibc/4.2.3
    ln -s ../../../Scrt1.o Scrt1.o
    ln -s ../../../crti.o crti.o
    ln -s ../../../crtn.o crtn.o
    ln -s ../../../crt1.o crt1.o
fi

# End of file
