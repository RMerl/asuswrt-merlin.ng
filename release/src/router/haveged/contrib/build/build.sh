#!/bin/sh
##
## Toggle between library and non-library builds. Fix messed up libtool environment
## Build and run haveged-devel sample
##
case "$1" in
nolib)
    sed -i.bak -e '/^##libtool_start##/,/^##libtool_end##/s,^,##,g' ../../configure.ac
    sed -i.bak -e '/^####nolibtool_start##/,/^####nolibtool_end##/s,^##,,g' \
        -e '/^##libtool_start##/,/^##libtool_end##/s,^,##,g' ../../src/Makefile.am
    cp nolib.spec ../../haveged.spec
;;
lib)
    sed -i.bak -e '/^####libtool_start##/,/^####libtool_end##/s,^##,,g' ../../configure.ac
    sed -i.bak -e '/^##nolibtool_start##/,/^##nolibtool_end##/s,^,##,g' \
        -e '/^####libtool_start##/,/^####libtool_end##/s,^##,,g' ../../src/Makefile.am
    cp lib.spec ../../haveged.spec
;;
new)
    cd ../..
    make distclean
    rm -rf autom4te.cache
    libtoolize --force --install
    autoreconf --force
    ./configure
;;
sample)
    echo "gcc -o havege_sample -DUSE_SOURCE -I../../src -Wall havege_sample.c ../../src/.libs/libhavege.a"
    gcc -o havege_sample -DUSE_SOURCE -I../../src -Wall havege_sample.c ../../src/.libs/libhavege.a
    echo "./havege_sample > /dev/null"
    ./havege_sample > /dev/null
;;

*)
    echo "usage: build [new|nolib|lib|sample]";
;;
esac

