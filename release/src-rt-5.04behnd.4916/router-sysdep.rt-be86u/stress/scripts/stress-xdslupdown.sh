#!/bin/ash

i=0;

num_loops=10

if [ $1 != "" ]; then
  echo "set num_loops to $1"
  num_loops=$1
fi

echo "Do dsl up/down $num_loops times"
echo ""

while [ $i -lt $num_loops ]; do
i=`expr $i + 1`
echo "==== starting loop $i ====="
date
echo ""
echo ""
date
echo "===> bring link down with xdslctl connection --down and wait 30 seconds"
xdslctl connection --down
echo "return val = $?"
echo ""
sleep 15
echo "--15--"
sleep 15 
date
echo "===> bring link up with xdslctl connection --up and wait 120 seconds"
xdslctl connection --up
echo "return val = $?"
echo ""
sleep 30 
echo "--30--"
sleep 30
echo "--60--"
sleep 30
echo "--90--"
sleep 30

done

# if the system survives the test, then its a pass
echo "PASS"


