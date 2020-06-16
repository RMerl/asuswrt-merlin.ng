#!/bin/bash
echo "$1 ($2, $3)..."

make clean 1>/dev/null 2>/dev/null

echo -n "building..."

if [ -f /proc/cpuinfo ]
then
  MAKE_JOBS=$(( ($(cat /proc/cpuinfo | grep -E '^processor[[:space:]]*:' | tail -n -1 | cut -d':' -f2) + 1) * 2 + 1 ))
else
  MAKE_JOBS=8
fi

CFLAGS="$2 $CFLAGS $4" EXTRALIBS="$5" make -j$MAKE_JOBS -f $3 all_test 1>gcc_1.txt 2>gcc_2.txt
mret=$?
cnt=$(wc -l < gcc_2.txt)
# ignore 1 line since ar prints to stderr instead of stdout and ar is called for
# $(LIBNAME)
if [[ $mret -ne 0 ]] || [[ $cnt -gt 1 ]]; then
   echo "build $1 failed! printing gcc_2.txt now for convenience"
   cat gcc_2.txt
   exit 1
fi

echo -n "testing..."

if [ -a test ] && [ -f test ] && [ -x test ]; then
   ((./test >test_std.txt 2>test_err.txt && ./tv_gen > tv.txt) && echo "$1 test passed." && echo "y" > testok.txt) || (echo "$1 test failed, look at test_err.txt or tv.txt" && exit 1)
   if find *_tv.txt -type f 1>/dev/null 2>/dev/null ; then
      for f in *_tv.txt; do
         # check for lines starting with '<' ($f might be a subset of notes/$f)
         difftroubles=$(diff -i -w -B $f notes/$f | grep '^<')
         if [ -n "$difftroubles" ]; then
            echo "FAILURE: $f"
            diff -i -w -B $f notes/$f
            echo "tv_gen $f failed" && rm -f testok.txt && exit 1
         else
            true
         fi
      done
   fi
fi


if [ -a testok.txt ] && [ -f testok.txt ]; then
   if [ "$LTC_COVERAGE" != "" ]; then
      ./coverage_more.sh > test_coverage_more.txt || exit 1
      lcov_opts="--capture --no-external --directory src -q"
      lcov_out=$(echo coverage_$1_$2_$3 | tr ' -=+' '_')".info"
      lcov $lcov_opts --output-file $lcov_out
   fi
   exit 0
fi
exit 1

# ref:         $Format:%D$
# git commit:  $Format:%H$
# commit time: $Format:%ai$
