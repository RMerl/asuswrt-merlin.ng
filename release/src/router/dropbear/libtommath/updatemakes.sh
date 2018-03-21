#!/bin/bash

bash genlist.sh > tmplist

perl filter.pl makefile tmplist
sed -e 's/ *$//' < tmp.delme > makefile
rm -f tmp.delme

perl filter.pl makefile.icc tmplist
sed -e 's/ *$//' < tmp.delme > makefile.icc
rm -f tmp.delme

perl filter.pl makefile.shared tmplist
sed -e 's/ *$//' < tmp.delme > makefile.shared
rm -f tmp.delme

perl filter.pl makefile.cygwin_dll tmplist
sed -e 's/ *$//' < tmp.delme > makefile.cygwin_dll
rm -f tmp.delme

perl filter.pl makefile.bcc tmplist
sed -e 's/\.o /.obj /g' -e 's/ *$//' < tmp.delme > makefile.bcc
rm -f tmp.delme

perl filter.pl makefile.msvc tmplist
sed -e 's/\.o /.obj /g' -e 's/ *$//' < tmp.delme > makefile.msvc
rm -f tmp.delme

rm -f tmplist

# ref:         $Format:%D$
# git commit:  $Format:%H$
# commit time: $Format:%ai$
