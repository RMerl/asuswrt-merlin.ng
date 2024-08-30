#!/bin/ash

i=0;

num_loops=10
sleep_secs=1

if [ $1 != "" ]; then
  echo "set num_loops to $1"
  num_loops=$1
fi

if [ $2 != "" ]; then
  echo "set sleep_secs to $2"
  sleep_secs=$2
fi

echo "cat /proc stuff $num_loops times"
echo ""

while [ $i -lt $num_loops ]; do
i=`expr $i + 1`
echo "==== starting loop $i ====="
date
ps
ls -R
cat /proc/meminfo
cat /proc/cpuinfo
cat /proc/net/dev
cat /proc/net/netstat

echo "===> done with loop $i ======="
date
echo "sleeping $sleep_secs"
sleep $sleep_secs

done

# if the system survives the test, then its a pass
echo "PASS"