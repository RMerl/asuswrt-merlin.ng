#!/bin/bash

runs=10
for i in $(seq -w "$runs"); do
  ./entest -vf <(../src/haveged -n 16384k -f -) > "${i}_entest.log"
done

fails=$(grep Fail ./*_entest.log | wc -l)

if (( fails > 2 )); then
  echo "Total $fails in $runs"
  grep Fail ./*_entest.log
  echo "Marking the whole test as failed"
  exit 255
else
  echo "Test passed!"
fi  

