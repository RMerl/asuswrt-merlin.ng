#!/bin/bash

#for i in $(seq -w 1000); do
#  ./entest -vf <(head -c 16M /dev/random) > "${i}_linux.log"
#done

mkdir chi_square
pushd chi_square || exit 1
for i in $(seq -w 1000); do
  ../entest -vf <(../../src/haveged -n 16384k -f -) > "${i}_haveged.log"
done

grep -Poh "Chi-Square: .*\(\K[0-9.]+" ./*haveged.log > ./chi.txt
R --vanilla <../examine_chi_square.R > examine_chi_square.summary
popd || exit 1
