#!/bin/bash
#
# return values of this script are:
#   0  success
# 128  a test failed
#  >0  the number of timed-out tests

set -e

if [ -f /proc/cpuinfo ]
then
  MAKE_JOBS=$(( ($(cat /proc/cpuinfo | grep -E '^processor[[:space:]]*:' | tail -n -1 | cut -d':' -f2) + 1) * 2 + 1 ))
else
  MAKE_JOBS=8
fi

ret=0
TEST_CFLAGS=""

_help()
{
  echo "Usage options for $(basename $0) [--with-cc=arg [other options]]"
  echo
  echo "Executing this script without any parameter will only run the default configuration"
  echo "that has automatically been determined for the architecture you're running."
  echo
  echo "    --with-cc=*             The compiler(s) to use for the tests"
  echo "        This is an option that will be iterated."
  echo
  echo "To be able to specify options a compiler has to be given."
  echo "All options will be tested with all MP_xBIT configurations."
  echo
  echo "    --with-{m64,m32,mx32}   The architecture(s) to build and test for,"
  echo "                            e.g. --with-mx32."
  echo "        This is an option that will be iterated, multiple selections are possible."
  echo "        The mx32 architecture is not supported by clang and will not be executed."
  echo
  echo "    --cflags=*              Give an option to the compiler,"
  echo "                            e.g. --cflags=-g"
  echo "        This is an option that will always be passed as parameter to CC."
  echo
  echo "    --make-option=*         Give an option to make,"
  echo "                            e.g. --make-option=\"-f makefile.shared\""
  echo "        This is an option that will always be passed as parameter to make."
  echo
  echo "    --with-low-mp           Also build&run tests with -DMP_{8,16,32}BIT."
  echo
  echo "Godmode:"
  echo
  echo "    --all                   Choose all architectures and gcc and clang as compilers"
  echo
  echo "    --help                  This message"
  exit 0
}

_die()
{
  echo "error $2 while $1"
  if [ "$2" != "124" ]
  then
    exit 128
  else
    echo "assuming timeout while running test - continue"
    ret=$(( $ret + 1 ))
  fi
}

_runtest()
{
  echo -ne " Compile $1 $2"
  make clean > /dev/null
  suffix=$(echo ${1}${2}  | tr ' ' '_')
  CC="$1" CFLAGS="$2 $TEST_CFLAGS" make -j$MAKE_JOBS test_standalone $MAKE_OPTIONS > /dev/null 2>gcc_errors_${suffix}.txt
  errcnt=$(wc -l < gcc_errors_${suffix}.txt)
  if [[ ${errcnt} -gt 1 ]]; then
    echo " failed"
    cat gcc_errors_${suffix}.txt
    exit 128
  fi
  echo -e "\rRun test $1 $2"
  local _timeout=""
  which timeout >/dev/null && _timeout="timeout --foreground 90"
  $_timeout ./test > test_${suffix}.txt || _die "running tests" $?
}

_banner()
{
  echo "uname="$(uname -a)
  [[ "$#" != "0" ]] && (echo $1=$($1 -dumpversion)) || true
}

_exit()
{
  if [ "$ret" == "0" ]
  then
    echo "Tests successful"
  else
    echo "$ret tests timed out"
  fi

  exit $ret
}

ARCHFLAGS=""
COMPILERS=""
CFLAGS=""
WITH_LOW_MP=""

while [ $# -gt 0 ];
do
  case $1 in
    "--with-m64" | "--with-m32" | "--with-mx32")
      ARCHFLAGS="$ARCHFLAGS ${1:6}"
    ;;
    --with-cc=*)
      COMPILERS="$COMPILERS ${1#*=}"
    ;;
    --cflags=*)
      CFLAGS="$CFLAGS ${1#*=}"
    ;;
    --make-option=*)
      MAKE_OPTIONS="$MAKE_OPTIONS ${1#*=}"
    ;;
    --with-low-mp)
      WITH_LOW_MP="1"
    ;;
    --all)
      COMPILERS="gcc clang"
      ARCHFLAGS="-m64 -m32 -mx32"
    ;;
    --help | -h)
      _help
    ;;
    *)
      echo "Ignoring option ${1}"
    ;;
  esac
  shift
done

# default to gcc if no compiler is defined but some other options
if [[ "$COMPILERS" == "" ]] && [[ "$ARCHFLAGS$MAKE_OPTIONS$CFLAGS" != "" ]]
then
   COMPILERS="gcc"
# default to gcc and run only default config if no option is given
elif [[ "$COMPILERS" == "" ]]
then
  _banner gcc
  _runtest "gcc" ""
  _exit
fi

archflags=( $ARCHFLAGS )
compilers=( $COMPILERS )

# choosing a compiler without specifying an architecture will use the default architecture
if [ "${#archflags[@]}" == "0" ]
then
  archflags[0]=" "
fi

_banner

for i in "${compilers[@]}"
do
  if [ -z "$(which $i)" ]
  then
    echo "Skipped compiler $i, file not found"
    continue
  fi
  compiler_version=$(echo "$i="$($i -dumpversion))
  if [ "$compiler_version" == "clang=4.2.1" ]
  then
    # one of my versions of clang complains about some stuff in stdio.h and stdarg.h ...
    TEST_CFLAGS="-Wno-typedef-redefinition"
  else
    TEST_CFLAGS=""
  fi
  echo $compiler_version

  for a in "${archflags[@]}"
  do
    if [[ $(expr "$i" : "clang") -ne 0 && "$a" == "-mx32" ]]
    then
      echo "clang -mx32 tests skipped"
      continue
    fi

    _runtest "$i $a" "$CFLAGS"
    [ "$WITH_LOW_MP" != "1" ] && continue
    _runtest "$i $a" "-DMP_8BIT $CFLAGS"
    _runtest "$i $a" "-DMP_16BIT $CFLAGS"
    _runtest "$i $a" "-DMP_32BIT $CFLAGS"
  done
done

_exit
