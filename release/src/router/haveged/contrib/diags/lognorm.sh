#!/bin/sh
src/haveged -n 16m -f i7.dat -r 4 -s i7.ln
ent/entest -vf i7.dat
nist/nist i7.dat
mv nist.out i7.out
src/haveged -n 16m -f xeon.dat -r 4 -s xeon.ln
ent/entest -vf xeon.dat
nist/nist xeon.dat
mv nist.out xeon.out
src/haveged -n 16m -f celeron.dat -r 4 -s celeron.ln
ent/entest -vf celeron.dat
nist/nist celeron.dat
mv nist.out celeron.out
