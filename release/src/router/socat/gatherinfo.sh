#! /bin/sh
# source: gatherinfo.sh
# Copyright Gerhard Rieger and contributors (see file CHANGES)
# Published under the GNU General Public License V.2, see file COPYING

#set -vx

# use this script after successful porting
# provide the platform name as argument with no dots, e.g. HPUX-11-0
# it generates the files:
# Config/Makefile.PLATFORM
# Config/config.PLATFORM.h
# Config/socat.PLATFORM.out
#
# Config/config.PLATFORM.log
# Config/compile.PLATFORM.log
# Config/test.PLATFORM.log

VERBOSE=
LOGGING=
INTERACTIVE=
CONFOPTS=
PLATFORM=
OUTPUT='>/dev/null'

# how to echo special characters?
if   [ `echo "x\c"` = "x" ]; then E=""
elif [ `echo -e "x\c"` = "x" ]; then E="-e"
fi

while [ -n "$1" ]; do
    case "$1" in
    -v) VERBOSE=1; shift;;	# tell about progress
    -d) LOGGING=1; shift;;	# show complete output
    -i) INTERACTIVE=1; shift;;	# diff and ask before overriding old files
    -*) CONFOPTS="$CONFOPTS $1"; shift;;
    *) PLATFORM="$1"; break;;
    esac
done

#if [ -z "$PLATFORM" ]; then
#  echo "please specify a configuration name, e.g. `uname -s`-`uname -r|tr '.' '-'`!" >&2; exit 1;
#fi

if [ $# -eq 0 ]; then
  echo $E "usage: $0 [-v] [-i] [configure options ...] platform" >&2
  echo $E "\t-v\t\tverbose (print actual command)" >&2
  echo $E "\t-d\t\tdump command outputs" >&2
  echo $E "\t-i\t\tinteractive (ask before overwriting something)" >&2
  echo $E "\tconfigure options\toptions for configure script, e.g. --disable-ip6" >&2
  echo $E "\tplatform\tdescribe your OS, e.g. `uname -s`-`uname -r|tr '.' '-'`" >&2
  exit 1
fi

case "$PLATFORM" in
*.*) echo "platform name must not contain '.'" >&2; exit 1;;
esac


# now, lets begin!

if [ -f Makefile ]; then
  COMMAND="make distclean"
  [ "$VERBOSE" ] && echo "$COMMAND"
  $COMMAND >/dev/null 2>&1 || echo "*** failed: $COMMAND" 1>&2
fi

# implicitly generates Makefile, config.h, config.log
COMMAND="./configure $CONFOPTS"
LOGFILE="compile.log"
[ "$VERBOSE" ] && echo "$COMMAND"
if [ "$LOGGING" ]; then
    { $COMMAND; echo "$?" >socat.rc; } 2>&1 |tee $LOGFILE;
    if [ `cat socat.rc` -ne 0 ]; then echo "*** failed: $COMMAND" 1>&2; exit 1; fi
else
    $COMMAND >$LOGFILE 2>&1 || { echo "*** failed: $COMMAND" 1>&2; exit 1; }
fi

COMMAND="make -k"
LOGFILE="compile.log"
[ "$VERBOSE" ] && echo "$COMMAND"
if [ "$LOGGING" ]; then
    { $COMMAND; echo "$?" >socat.rc; } 2>&1 |tee -a $LOGFILE;
    if [ `cat socat.rc` -ne 0 ]; then echo "*** failed: $COMMAND" 1>&2; exit 1; fi
else    
    $COMMAND >>$LOGFILE 2>&1 || { echo "*** failed: $COMMAND" 1>&2; exit 1; }
fi

# generates socat.out
COMMAND="make info"
[ "$VERBOSE" ] && echo "$COMMAND"
$COMMAND >/dev/null || echo "*** failed: $COMMAND" 1>&2

COMMAND="./test.sh"
LOGFILE="test.log"
[ "$VERBOSE" ] && echo "$COMMAND"
if [ "$LOGGING" ]; then
    { $COMMAND; echo "$?" >socat.rc; } 2>&1 |tee $LOGFILE;
    if [ `cat socat.rc` -ne 0 ]; then
	echo "*** failed: $COMMAND" 1>&2
	if [ `cat socat.rc` -ge 128 ]; then
	    exit 1
	fi
    fi
else    
    $COMMAND >$LOGFILE 2>&1 || echo "*** failed: $COMMAND" 1>&2
fi

FILES=

b=Makefile; e=; f=$b; p=Config/$b.$PLATFORM
if [ "$INTERACTIVE" -a -f $p ]; then
  if ! diff $p $f; then
    cp -pi $f $p
  fi
else
  cp -p $f $p
fi
FILES="$p"

b=config; e=h; f=$b.$e; p=Config/$b.$PLATFORM.$e
if [ "$INTERACTIVE" -a -f $p ]; then
  if ! diff $p $f; then
    cp -pi $f $p
  fi
else
  cp -p $f $p
fi
FILES="$FILES $p"

b=socat; e=out; f=$b.$e; p=Config/$b.$PLATFORM.$e
if [ "$INTERACTIVE" -a -f $p ]; then
  if ! diff $p $f; then
    cp -pi $f $p
  fi
else
  cp -p $f $p
fi
FILES="$FILES $p"

b=config; e=log; f=$b.$e; p=Config/$b.$PLATFORM.$e
if [ "$INTERACTIVE" -a -f $p ]; then
  if ! diff $p $f; then
    cp -pi $f $p
  fi
else
  cp -p $f $p
fi
FILES="$FILES $p"

b=compile; e=log; f=$b.$e; p=Config/$b.$PLATFORM.$e
if [ "$INTERACTIVE" -a -f $p ]; then
  if ! diff $p $f; then
    cp -pi $f $p
  fi
else
  cp -p $f $p
fi
FILES="$FILES $p"

b=test; e=log; f=$b.$e; p=Config/$b.$PLATFORM.$e
if [ "$INTERACTIVE" -a -f $p ]; then
  if ! diff $p $f; then
    cp -pi $f $p
  fi
else
  cp -p $f $p
fi
FILES="$FILES $p"

echo "output files:"
echo "$FILES"
