#!/bin/bash

if [ $# -lt 3 ]
then
  echo "usage is: ${0##*/} <which makefile and other make options> <additional defines> <path to math provider>"
  echo "e.g. \"${0##*/} \"makefile -j9\" \"-DUSE_LTM -DLTM_DESC -I../libtommath\" ../libtommath/libtommath.a\""
  echo "to create aggregate coverage: pre-pend with LTC_COVERAGE=1"
  exit -1
fi

# date
echo "date="`date`

# check sources
bash check_source.sh "CHECK_SOURCES" " " "$1" "$2" "$3" || exit 1

mk="$1"

[ "$LTC_COVERAGE" != "" ] && mk="$mk COVERAGE=1"

# stock build
bash run.sh "STOCK" " " "$mk" "$2" "$3" || exit 1

# EASY build
bash run.sh "EASY" "-DLTC_EASY" "$mk" "$2" "$3" || exit 1

# SMALL code
bash run.sh "SMALL" "-DLTC_SMALL_CODE" "$mk" "$2" "$3" || exit 1

# NOTABLES
bash run.sh "NOTABLES" "-DLTC_NO_TABLES" "$mk" "$2" "$3" || exit 1

# SMALL+NOTABLES
bash run.sh "SMALL+NOTABLES" "-DLTC_SMALL_CODE -DLTC_NO_TABLES" "$mk" "$2" "$3" || exit 1

# CLEANSTACK
bash run.sh "CLEANSTACK" "-DLTC_CLEAN_STACK" "$mk" "$2" "$3" || exit 1

# CLEANSTACK + SMALL
bash run.sh "CLEANSTACK+SMALL" "-DLTC_SMALL_CODE -DLTC_CLEAN_STACK" "$mk" "$2" "$3" || exit 1

# CLEANSTACK + NOTABLES
bash run.sh "CLEANSTACK+NOTABLES" "-DLTC_NO_TABLES -DLTC_CLEAN_STACK" "$mk" "$2" "$3" || exit 1

# CLEANSTACK + NOTABLES + SMALL
bash run.sh "CLEANSTACK+NOTABLES+SMALL" "-DLTC_NO_TABLES -DLTC_CLEAN_STACK -DLTC_SMALL_CODE" "$mk" "$2" "$3" || exit 1

# NO_FAST
bash run.sh "NO_FAST" "-DLTC_NO_FAST" "$mk" "$2" "$3" || exit 1

# NO_FAST + NOTABLES
bash run.sh "NO_FAST+NOTABLES" "-DLTC_NO_FAST -DLTC_NO_TABLES" "$mk" "$2" "$3" || exit 1

# NO_ASM
bash run.sh "NO_ASM" "-DLTC_NO_ASM" "$mk" "$2" "$3" || exit 1

# NO_TIMING_RESISTANCE
bash run.sh "NO_TIMING_RESISTANCE" "-DLTC_NO_ECC_TIMING_RESISTANT -DLTC_NO_RSA_BLINDING" "$mk" "$2" "$3" || exit 1

# CLEANSTACK+NOTABLES+SMALL+NO_ASM+NO_TIMING_RESISTANCE
bash run.sh "CLEANSTACK+NOTABLES+SMALL+NO_ASM+NO_TIMING_RESISTANCE" "-DLTC_CLEAN_STACK -DLTC_NO_TABLES -DLTC_SMALL_CODE -DLTC_NO_ECC_TIMING_RESISTANT -DLTC_NO_RSA_BLINDING" "$mk" "$2" "$3" || exit 1

# test build with no testing
bash testbuild.sh "NOTEST" "-DLTC_NO_TEST" "$mk" "$2" "$3" || exit 1

# test build with no file routines
bash testbuild.sh "NOFILE" "-DLTC_NO_FILE" "$mk" "$2" "$3" || exit 1

# ref:         $Format:%D$
# git commit:  $Format:%H$
# commit time: $Format:%ai$
