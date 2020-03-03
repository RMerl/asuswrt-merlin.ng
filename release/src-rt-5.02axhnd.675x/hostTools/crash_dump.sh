#!/bin/bash

#################################################################
# (1) This script currently only decodes addresses in wl.ko.
# (2) How to use: put console log in the project root directory,
#     for example, CommEngine/console.log; then from there
#     execute: "hostTools/crash_dump.sh console.log".
# 

process_crash_dump()
{
  log=$1
  wl_driver=$2
  dos2unix $log

  sed -ne "/^Oops/,/^Kernel panic/ p" < $log > $log.tmp
  core_addr=`sed -ne "s/wl(.*core_addr(\(........\).*)/0x\1/p" < $log.tmp`
  epc=`sed -ne "s/^epc[ \t]*: \(........\).*/0x\1/p" < $log.tmp`
  ra=`sed -ne "s/^ra[ \t]*: \(........\).*/0x\1/p" < $log.tmp`

  echo "add-symbol-file $wl_driver $core_addr" > gdb.tmp
  echo "l *$epc" >> gdb.tmp
  echo "l *$ra" >> gdb.tmp
} 

if [ -z "$1" ] || [ ! -f "$1" ] ; then
  echo "Usage: crash_dump.sh LOG_FILE_NAME"
  exit
fi

PROFILE=`find targets -name vmlinux | sed -e "s?targets/??" -e "s?/.*??" -e "q"`

if [ -z "$PROFILE" ]; then
  echo "PROFILE is not defined!"
  exit;
fi
 
gdb=/opt/toolchains/uclibc-crosstools/bin/mips-linux-uclibc-gdb
driver=./targets/$PROFILE/modules/lib/modules/2.6.21.5/extra/wl.ko

process_crash_dump $1 $driver
$gdb -q -n -x gdb.tmp

# end of file

